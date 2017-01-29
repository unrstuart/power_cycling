#include <cassert>
#include <cmath>
#include <cstdio>

#include <algorithm>

#include "si_var.h"

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
#if 0
    printf("%2d power=%s speed_guess=%s comp=%s diff=%s\n", i,
           power.ToString().c_str(), speed.ToString().c_str(),
           computed_power.ToString().c_str(),
           (power - computed_power).ToString().c_str());
#endif
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
  return 0;
}

}  // namespace
}  // namespace cycling

int main(int argc, char** argv) { return cycling::Main(); }
