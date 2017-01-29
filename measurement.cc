#include "measurement.h"

#include <cstdio>
#include <string>

namespace cycling {

namespace {

std::string DtoA(const double d) {
  char buf[50];
  sprintf(buf, "%.3f", d);
  return buf;
}

std::string ItoA(const int i) {
  char buf[21];
  sprintf(buf, "%d", i);
  return buf;
}

}  // namespace

std::string Measurement::ToString() const {
  switch (type_) {
    case NO_TYPE:
      return "null";
    case HEART_RATE:
      return DtoA(value_.coef()) + " bpm";
    case HRV:
      return value_.ToString();
    case SPEED:
      return value_.ToString();
    case CADENCE:
      return DtoA(value_.coef()) + " rpm";
    case POWER:
      return value_.ToString();
    case GEAR:
      return "gear " + ItoA(static_cast<int>(value_.coef()));
    case TOTAL_DISTANCE:
      return DtoA(value_.coef() / 1000.0) + " km";
    case TOTAL_CALORIES:
      return DtoA(value_.coef() / 4.814) + " total kCal";
  }
}

}  // namespace cycling
