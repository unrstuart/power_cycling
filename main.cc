#include <cassert>
#include <cmath>
#include <cstdio>

#include <algorithm>
#include <chrono>
#include <map>
#include <sstream>
#include <vector>

#include "grapher.h"
#include "measurement.h"
#include "si_var.h"
#include "string_buffer.h"
#include "tcx_util.h"
#include "time_series.h"

namespace cycling {
namespace {

const double ROLLING_COEF = 0.005;
const double DRAG_COEF = 0.63;
const double DRIVETRAIN_LOSS = 3;
const SiVar GRAVITY = SiVar::Meter() / SiVar::Second().Power(2) * 9.8067;
const SiVar AIR_DENSITY = 1.226 * SiVar::Kilogram() / SiVar::Meter().Power(3);
const SiVar BIKE_WEIGHT = 9 * SiVar::Kilogram();

SiVar ComputeFrontalArea(const SiVar& weight) {
  assert(weight.unit() == SiUnit::Kilogram());
  const double c = std::min(100.0, std::max(50.0, weight.coef())) / 50.0;
  return (c * 0.3) * SiVar::Meter().Power(2);
}

SiVar ComputeRolling(const SiVar rider_weight, const SiVar& gravity,
                     const SiVar& bike_weight, const double rolling_coef) {
  return gravity * (rider_weight + bike_weight) *
         std::cos(std::atan(gravity.coef() / 100)) * rolling_coef;
}

SiVar ComputeDrag(const SiVar& frontal_area, const SiVar& speed,
                  const SiVar& air_density, const double drag_coef) {
  return 0.5 * frontal_area * drag_coef * air_density * speed * speed;
}

SiVar ComputePower(const SiVar& rider_weight, const SiVar& bike_weight,
                   const SiVar& gravity, const SiVar& frontal_area,
                   const SiVar& speed, const SiVar& air_density,
                   const double drivetrain_loss, const double drag_coef,
                   const double rolling_coef) {
  const SiVar rolling_force =
      ComputeRolling(rider_weight, gravity, bike_weight, rolling_coef);
  const SiVar drag_force =
      ComputeDrag(frontal_area, speed, air_density, drag_coef);
  const SiVar wheel_power = (drag_force + rolling_force) * speed;
  return wheel_power / (1.0 - drivetrain_loss / 100.0);
}

SiVar ComputeSpeed(const SiVar& power, const SiVar& rider_weight,
                   const SiVar& bike_weight, const SiVar& gravity,
                   const SiVar& air_density, const double rolling_coef,
                   const double drag_coef, const double drivetrain_loss) {
  const SiVar frontal_area = ComputeFrontalArea(rider_weight);

  SiVar speed = 35 * SiVar::KilometersPerHour();
  SiVar adjust = speed;
  const SiVar power_epsilon = 0.0001 * SiVar::Watt();
  for (int i = 0; i < 25; ++i) {
    const SiVar computed_power =
        ComputePower(rider_weight, bike_weight, gravity, frontal_area, speed,
                     air_density, drivetrain_loss, drag_coef, rolling_coef);
    if ((computed_power - power).Abs() < power_epsilon) break;
    if (computed_power > power) {
      speed -= adjust;
    } else {
      speed += adjust;
    }
    adjust /= 2.0;
  }
  return speed;
}

void DumpTimeSeries(const std::string& file_path) {
  using Duration = std::chrono::system_clock::duration;

  std::unique_ptr<TimeSeries> series = ParseTcxFile(file_path);
  if (!series) {
    std::cerr << "Couldn't read " << file_path << ", skipping." << std::endl;
    return;
  }
  FILE* fp = fopen((file_path + "-time_series.out").c_str(), "w");

  const Duration kWindow = std::chrono::seconds(300);
  const Duration kIncrement = std::chrono::seconds(1);
  const Duration kLookBehind = std::chrono::seconds(5);
  const int kNumFrames = 30;
  const int kNumSamples =
      series->EndTime().time_since_epoch().count() / 1000000 -
      series->BeginTime().time_since_epoch().count() / 1000000;
  Grapher grapher(kWindow, kIncrement, kLookBehind);

  int str_size;
  const char* kCaptions[] = {
      "Heart Rate", "Power", "Speed", "Cadence",
  };
  const int num_fields = sizeof(kCaptions) / sizeof(kCaptions[0]);
  fwrite(&num_fields, sizeof(num_fields), 1, fp);
  for (const char* caption : kCaptions) {
    str_size = strlen(caption);
    fwrite(&str_size, sizeof(str_size), 1, fp);
    fwrite(caption, str_size, 1, fp);
  }

  const int total_frames = kNumFrames * kNumSamples;
  fwrite(&total_frames, sizeof(total_frames), 1, fp);
  int last = -1;
  for (int i = 0; i < kNumSamples; ++i) {
    double num = kNumSamples;
    StringBuffer buffer;
    if (std::floor(last / num * 100) < std::floor(i / num * 100)) {
      printf("%5.1f%%\r", i / num * 100);
      fflush(stdout);
      last = i;
    }
    for (int frame = 0; frame < kNumFrames; ++frame) {
      for (const auto m : {Measurement::HEART_RATE, Measurement::POWER,
                           Measurement::SPEED, Measurement::CADENCE}) {
        Grapher::Graph graph =
            grapher.Plot(*series, series->BeginTime() + std::chrono::seconds(i),
                         m, 1.0, frame / static_cast<double>(kNumFrames));

        int num_labels = graph.labels.size();
        int num_points = graph.points.size();

        buffer.Add(num_labels).Add(num_points);
        buffer.Add(graph.labels.data(), graph.labels.size());
        buffer.Add(graph.points.data(), graph.points.size());
      }
    }
    fwrite(buffer.str(), buffer.size(), 1, fp);
  }
  printf("100.0%%\n");
  fclose(fp);
}

int Main(int argc, char** argv) {
  const SiVar rider_weight = 85 * SiVar::Kilogram();

  for (int power_coef = 10; power_coef <= 500; power_coef += 10) {
    const SiVar power = power_coef * SiVar::Watt();
    const SiVar speed =
        ComputeSpeed(power, rider_weight, BIKE_WEIGHT, GRAVITY, AIR_DENSITY,
                     ROLLING_COEF, DRAG_COEF, DRIVETRAIN_LOSS);
    printf("power: %9s speed: %11s\n", power.ToString().c_str(),
           speed.ToString().c_str());
  }

  for (int i = 1; i < argc; ++i) {
    std::cerr
        << "Loading " << argv[i]
        << " and converting HR, speed, cadence, and power to time series.\n";
    DumpTimeSeries(argv[i]);
  }

  return 0;
}

}  // namespace
}  // namespace cycling

int main(int argc, char** argv) { return cycling::Main(argc, argv); }
