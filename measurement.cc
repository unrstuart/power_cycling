#include "measurement.h"

#include <cassert>
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

Measurement::Measurement(const Type type, const double coef) : type_(type) {
  switch (type) {
    case NO_TYPE:
    case DEGREES_LATITUDE:
    case DEGREES_LONGITUDE:
    case HEART_RATE:
    case HRV:
    case CADENCE:
    case GEAR:
      value_ = SiVar(SiBaseUnit::UNITLESS, coef);
      break;
    case SPEED:
      value_ = SiVar(SiUnit::MetersPerSecond(), coef);
      break;
    case POWER:
      value_ = SiVar(SiUnit::Watt(), coef);
      break;
    case ALTITUDE:
    case INCREMENTAL_DISTANCE:
    case TOTAL_DISTANCE:
      value_ = SiVar(SiUnit::Meter(), coef);
      break;
    case TOTAL_JOULES:
      value_ = SiVar(SiUnit::Joule(), coef);
      break;
    case NUM_MEASUREMENTS:
      assert(false);
      break;
  }
}

Measurement::Measurement(const Type type, const SiVar& var)
    : type_(type), value_(var) {
  switch (type) {
    case NO_TYPE:
    case DEGREES_LATITUDE:
    case DEGREES_LONGITUDE:
    case HEART_RATE:
    case HRV:
    case CADENCE:
    case GEAR:
      assert(value_.unit() == SiBaseUnit::UNITLESS);
      break;
    case SPEED:
      assert(value_.unit() == SiUnit::MetersPerSecond());
      break;
    case POWER:
      assert(value_.unit() == SiUnit::Watt());
      break;
    case INCREMENTAL_DISTANCE:
    case TOTAL_DISTANCE:
    case ALTITUDE:
      assert(value_.unit() == SiUnit::Meter());
      break;
    case TOTAL_JOULES:
      assert(value_.unit() == SiUnit::Joule());
      break;
    case NUM_MEASUREMENTS:
      assert(false);
      break;
  }
}

std::string Measurement::ToString() const {
  switch (type_) {
    case NO_TYPE:
      return "null";
    case DEGREES_LATITUDE:
      return DtoA(value_.coef()) + " deg lat";
    case DEGREES_LONGITUDE:
      return DtoA(value_.coef()) + " deg lon";
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
    case ALTITUDE:
    case INCREMENTAL_DISTANCE:
      return value_.ToString();
    case TOTAL_DISTANCE:
      return DtoA(value_.coef() / 1000.0) + " km";
    case TOTAL_JOULES:
      return DtoA(value_.coef() / 4.814) + " total kCal";
    case NUM_MEASUREMENTS:
      assert(false);
      return "";
  }
}

bool Measurement::operator==(const Measurement& rhs) const {
  return type_ == rhs.type_ && value_ == rhs.value_;
}
bool Measurement::operator!=(const Measurement& rhs) const {
  return !(*this == rhs);
}
bool Measurement::operator<(const Measurement& rhs) const {
  if (type_ != rhs.type_) return type_ < rhs.type_;
  return value_ < rhs.value_;
}
bool Measurement::operator>(const Measurement& rhs) const {
  return rhs < *this;
}
bool Measurement::operator<=(const Measurement& rhs) const {
  return !(*this > rhs);
}
bool Measurement::operator>=(const Measurement& rhs) const {
  return !(*this < rhs);
}

}  // namespace cycling
