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

SiVar ComputeRolling(const SiVar rider_weight) {
  return GRAVITY * (rider_weight + BIKE_WEIGHT) *
         std::cos(std::atan(GRAVITY.coef() / 100)) * ROLLING_COEF;
}

SiVar ComputeDrag(const SiVar& frontal_area, const SiVar& speed) {
  return 0.5 * frontal_area * DRAG_COEF * AIR_DENSITY * speed * speed;
}

int Main() {
  const SiVar rider_weight = 85 * SiVar::Kilogram();
  const SiVar frontal_area = ComputeFrontalArea(rider_weight);
  printf("front: %s\n", frontal_area.ToString().c_str());

  for (int speed_coef = 5; speed_coef <= 50; speed_coef += 5) {
    const SiVar speed = speed_coef * SiVar::KilometersPerHour();
    const SiVar rolling_force = ComputeRolling(rider_weight);
    const SiVar drag_force = ComputeDrag(frontal_area, speed);
    const SiVar drag_power = drag_force * speed;
    const SiVar rolling_power = rolling_force * speed;
    const SiVar wheel_power = (drag_force + rolling_force) * speed;
    const SiVar total_power = wheel_power / (1.0 - DRIVETRAIN_LOSS / 100.0);

    printf("speed: %7.3f km/h drag: %9s rolling: %9s total: %9s\n",
           speed.coef() * 3.6, drag_power.ToString().c_str(),
           rolling_power.ToString().c_str(), total_power.ToString().c_str());
  }
  return 0;
}

}  // namespace
}  // namespace cycling

int main(int argc, char** argv) { return cycling::Main(); }
