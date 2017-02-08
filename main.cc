#include <cassert>
#include <cmath>
#include <cstdio>

#include <algorithm>
#include <chrono>
#include <map>
#include <vector>

#include "grapher.h"
#include "measurement.h"
#include "si_var.h"
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

void DumpTimeSeries() {
  using Duration = std::chrono::system_clock::duration;
  using Time = std::chrono::system_clock::time_point;

  const double kMinHr = 39;
  const double kMaxHr = 185;
  const double kSamplePeriod = 100;
  const int kNumSamples = 1200;
  const Duration kWindow = std::chrono::seconds(30);
  const Duration kIncrement = std::chrono::seconds(1);
  const Duration kLookBehind = std::chrono::seconds(3);
  const int kNumFrames = 30;

  TimeSeries time_series;
  const Time start = std::chrono::system_clock::from_time_t(0);

  for (int i = 0; i < kNumSamples; ++i) {
    double hr;
    if (false) {
      hr = i;
    } else {
      hr = kMinHr + (kMaxHr - kMinHr) / 2.0 +
           (kMaxHr - kMinHr) * std::sin(M_PI * (i / kSamplePeriod) * 2) / 2.0;
    }
    time_series.Add(TimeSample(start + std::chrono::milliseconds(i * 1000),
                               Measurement(Measurement::HEART_RATE, hr)));
  }

  FILE* fp = fopen("time_series.out", "w");

  Grapher grapher(kWindow, kIncrement, kLookBehind);

  for (int i = 0; i < kNumSamples; ++i) {
    for (int frame = 0; frame < kNumFrames; ++frame) {
      Grapher::Graph graph = grapher.Plot(
          time_series, start + std::chrono::seconds(i), Measurement::HEART_RATE,
          1.0, frame / static_cast<double>(kNumFrames));
      int num_labels = graph.labels.size();
      int num_points = graph.points.size();

      fwrite(&num_labels, sizeof(num_labels), 1, fp);
      fwrite(&num_points, sizeof(num_points), 1, fp);
      for (const auto& label : graph.labels) {
        fwrite(&label.first, sizeof(label.first), 1, fp);
        fwrite(&label.second, sizeof(label.second), 1, fp);
      }
      for (const auto& point : graph.points) {
        fwrite(&point.x, sizeof(point.x), 1, fp);
        fwrite(&point.y, sizeof(point.y), 1, fp);
      }
    }
  }
  fclose(fp);
}

int Main() {
  const SiVar rider_weight = 85 * SiVar::Kilogram();

  for (int power_coef = 10; power_coef <= 500; power_coef += 10) {
    const SiVar power = power_coef * SiVar::Watt();
    const SiVar speed =
        ComputeSpeed(power, rider_weight, BIKE_WEIGHT, GRAVITY, AIR_DENSITY,
                     ROLLING_COEF, DRAG_COEF, DRIVETRAIN_LOSS);
    printf("power: %9s speed: %11s\n", power.ToString().c_str(),
           speed.ToString().c_str());
  }

  DumpTimeSeries();

  return 0;
}

}  // namespace
}  // namespace cycling

int main(int argc, char** argv) { return cycling::Main(); }
