#ifndef __SI_VAR_H__
#define __SI_VAR_H__

#include <string>

#include "si_unit.h"

namespace cycling {

// Represents a numerical value with units (which may be unitless), e.g 9.8
// m/s^2. For a full list of possible units, please see SiBaseUnit. This class
// cannot hold a mixture of different units in an expression (e.g. 4 m + 6 s),
// it can only hold one coefficient combined with one collection of units.
class SiVar {
 public:
  static SiVar Gram();
  static SiVar Kilogram();
  static SiVar Meter();
  static SiVar Kilometer();
  static SiVar Second();
  static SiVar Minute();
  static SiVar Hour();
  static SiVar Newton();
  static SiVar Joule();
  static SiVar Watt();
  static SiVar MetersPerSecond();
  static SiVar KilometersPerHour();

  std::string ToString() const;

  SiVar() : SiVar(SiBaseUnit::UNITLESS) {}
  SiVar(const double coef) : SiVar(SiBaseUnit::UNITLESS, coef) {}
  SiVar(const SiUnit& unit) : SiVar(unit, /*coef=*/0) {}
  SiVar(const SiUnit& unit, const double coef);
  SiVar(const SiVar&) = default;
  SiVar(SiVar&&) = default;

  SiVar& operator=(const SiVar&) = default;
  SiVar& operator=(SiVar&&) = default;

  SiVar operator+(const SiVar& var) const;
  SiVar& operator+=(const SiVar& var);
  SiVar operator-(const SiVar& var) const;
  SiVar& operator-=(const SiVar& var);
  SiVar operator*(const SiVar& rhs) const;
  SiVar operator*(const double d) const;
  friend SiVar operator*(const SiUnit& unit, const SiVar& var);
  friend SiVar operator*(const double d, const SiVar& var);
  SiVar& operator*=(const double d);
  SiVar& operator*=(const SiVar& v);
  SiVar operator/(const SiVar& unit) const;
  SiVar operator/(const double d) const;
  friend SiVar operator/(const SiUnit& unit, const SiVar& var);
  friend SiVar operator/(const double d, const SiVar& unit);
  SiVar& operator/=(const double d);
  SiVar& operator/=(const SiVar& v);
  SiVar Invert() const;
  SiVar Power(const int power) const;
  SiVar Abs() const;

  bool operator<(const SiVar& rhs) const { return Compare(rhs) < 0; }
  bool operator>(const SiVar& rhs) const { return Compare(rhs) > 0; }
  bool operator==(const SiVar& rhs) const { return Compare(rhs) == 0; }
  bool operator!=(const SiVar& rhs) const { return Compare(rhs) != 0; }

  double coef() const { return coef_; }
  const SiUnit& unit() const { return unit_; }

  void set_coef(const double coef) { coef_ = coef; }
  void set_unit(const SiUnit& unit) { unit_ = unit; }

  friend std::ostream& operator<<(std::ostream& out, const SiVar& rhs) {
    return out << rhs.ToString();
  }

 private:
  int Compare(const SiVar& rhs) const;

  SiUnit unit_;
  double coef_;
};

}  // namespace cycling

#endif  // __SI_VAR_H__
