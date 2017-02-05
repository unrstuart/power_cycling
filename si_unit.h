#ifndef __SI_UNIT_H__
#define __SI_UNIT_H__

#include <iostream>
#include <map>

#include "si_base_unit.h"

namespace cycling {

// Holds a collection of SI base units and their exponents in order to represent
// coefficientless measurements (e.g. kg * m/s^2).
class SiUnit {
 public:
  // Constructs a new SI unit, filtering out UNITLESS, and units with a zero
  // exponent.
  SiUnit() = default;
  SiUnit(const SiUnit&) = default;
  SiUnit(SiUnit&&) = default;
  SiUnit(const std::map<SiBaseUnit, int>& units_and_exps);
  SiUnit(SiBaseUnit unit);
  SiUnit(const SiBaseUnit unit, const int exp);
  ~SiUnit() = default;

  SiUnit& operator=(const SiUnit&) = default;
  SiUnit& operator=(SiUnit&&) = default;

  SiUnit Invert() const;
  SiUnit Power(const int exp) const;
  SiUnit operator*(const SiUnit& rhs) const;
  SiUnit operator*=(const SiUnit& rhs);
  SiUnit operator/(const SiUnit& rhs) const;
  SiUnit operator/=(const SiUnit& rhs);
  bool operator==(const SiUnit& rhs) const;
  bool operator!=(const SiUnit& rhs) const { return !operator==(rhs); }

  std::string ToString() const;

  static SiUnit Unitless();
  static SiUnit Kilogram();
  static SiUnit Meter();
  static SiUnit Second();
  static SiUnit Newton();
  static SiUnit Joule();
  static SiUnit Watt();
  static SiUnit MetersPerSecond();

  using iterator = std::map<SiBaseUnit, int>::const_iterator;
  iterator begin() const { return units_.cbegin(); }
  iterator end() const { return units_.cend(); }
  const std::map<SiBaseUnit, int>& units() const { return units_; }

  friend std::ostream& operator<<(std::ostream& out, const SiUnit& unit) {
    return out << unit.ToString();
  }

 private:
  // Maps the base unit to an exponent (e.g. {METER, 2} would be square meters).
  std::map<SiBaseUnit, int> units_;
};

}  // namespace cycling

#endif  // __SI_UNIT_H__
