#include "si_var.h"
#include "si_var.h"

#include <cassert>
#include <cmath>
#include <string>

#include "si_base_unit.h"

namespace cycling {

SiVar SiVar::Gram() { return Kilogram() / 1000.0; }
SiVar SiVar::Kilogram() { return SiUnit(SiBaseUnit::KILOGRAM); }
SiVar SiVar::Meter() { return SiUnit(SiBaseUnit::METER); }
SiVar SiVar::Kilometer() { return SiVar(SiBaseUnit::METER, 1000.0); }
SiVar SiVar::Second() { return SiUnit(SiBaseUnit::SECOND); }
SiVar SiVar::Minute() { return SiVar(SiBaseUnit::SECOND, 60); }
SiVar SiVar::Hour() { return Minute() * 60; }
SiVar SiVar::Newton() { return Kilogram() * Meter() * Second().Power(-2); }
SiVar SiVar::Joule() { return Newton() * Meter(); }
SiVar SiVar::Watt() { return Joule() / Second(); }
SiVar SiVar::MetersPerSecond() { return Meter() / Second(); }
SiVar SiVar::KilometersPerHour() { return Kilometer() / Hour(); }

std::string SiVar::ToString() const {
  if (coef_ == 0) return "0";
  char buf[200];
  sprintf(buf, "%.3f", coef_);
  const std::string units = unit_.ToString();
  if (units.empty()) return units;
  return buf + (" " + units);
}

SiVar::SiVar(const SiUnit& unit) : unit_(unit), coef_(1.0) {}

SiVar SiVar::operator+(const SiVar& var) const {
  assert(unit_ == var.unit_);
  return SiVar(unit_, coef_ + var.coef_);
}

SiVar& SiVar::operator+=(const SiVar& var) { return *this = *this + var; }

SiVar SiVar::operator-(const SiVar& var) const {
  assert(unit_ == var.unit_);
  return SiVar(unit_, coef_ - var.coef_);
}

SiVar& SiVar::operator-=(const SiVar& var) { return *this = *this - var; }

SiVar SiVar::operator*(const SiVar& rhs) const {
  return SiVar(unit_ * rhs.unit_, coef_ * rhs.coef_);
}

SiVar SiVar::operator*(const double d) const { return SiVar(unit_, coef_ * d); }

SiVar operator*(const SiUnit& unit, const SiVar& var) {
  return SiVar(unit * var.unit_, var.coef_);
}

SiVar operator*(const double d, const SiVar& var) {
  return SiVar(var.unit_, var.coef_ * d);
}

SiVar SiVar::operator/(const SiVar& unit) const {
  return *this * unit.Invert();
}

SiVar operator/(const SiUnit& unit, const SiVar& var) {
  return SiVar(unit * var.unit_.Invert(), 1.0 / var.coef_);
}

SiVar operator/(const double d, const SiVar& unit) {
  return SiVar(unit.unit_.Invert(), d / unit.coef_);
}

SiVar SiVar::operator/(const double d) const { return SiVar(unit_, coef_ / d); }

SiVar& SiVar::operator/=(const double d) {
  coef_ /= d;
  return *this;
}

SiVar SiVar::Invert() const { return Power(-1); }

SiVar SiVar::Power(const int power) const {
  return SiVar(unit_.Power(power), std::pow(coef_, power));
}

SiVar SiVar::Abs() const {
  if (coef_ >= 0) return *this;
  return SiVar(unit_, -coef_);
}

SiVar::SiVar(const SiUnit& unit, const double coef)
    : unit_(unit), coef_(coef) {}

int SiVar::Compare(const SiVar& rhs) const {
  assert(unit_ == rhs.unit_);
  if (coef_ < rhs.coef_) {
    return -1;
  } else if (coef_ > rhs.coef_) {
    return 1;
  }
  return 0;
}

}  // namespace cycling
