#include "si_unit.h"

#include <set>

namespace cycling {

SiUnit::SiUnit(const std::map<SiBaseUnit, int> units_and_exps)
    : units_(units_and_exps) {
  auto it = units_.find(SiBaseUnit::UNITLESS);
  if (it != units_.end()) units_.erase(it);
  std::set<SiBaseUnit> to_remove;
  for (const auto& p : units_) {
    if (p.second == 0) to_remove.insert(p.first);
  }
  for (const auto unit : to_remove) {
    units_.erase(units_.find(unit));
  }
}

SiUnit::SiUnit(SiBaseUnit unit) : SiUnit(unit, 1) {}

SiUnit::SiUnit(const SiBaseUnit unit, const int exp) {
  if (exp == 0) return;
  if (unit == SiBaseUnit::UNITLESS) return;
  units_[unit] = exp;
}

SiUnit SiUnit::Invert() const { return Power(-1); }

SiUnit SiUnit::operator*(const SiUnit& rhs) const {
  SiUnit ret = *this;
  for (const auto& p : rhs.units_) {
    auto it = ret.units_.find(p.first);
    if (it != ret.units_.end()) {
      it->second += p.second;
      if (it->second == 0) ret.units_.erase(it);
    } else if (p.second != 0 && p.first != SiBaseUnit::UNITLESS) {
      ret.units_[p.first] = p.second;
    }
  }
  return ret;
}

SiUnit SiUnit::operator/(const SiUnit& rhs) const {
  return *this * rhs.Invert();
}

SiUnit SiUnit::Power(const int exp) const {
  if (exp == 0) return SiBaseUnit::UNITLESS;
  SiUnit ret = *this;
  for (auto& p : ret.units_) p.second *= exp;
  return ret;
}

bool SiUnit::operator==(const SiUnit& rhs) const {
  return units_ == rhs.units_;
}

std::string SiUnit::ToString() const {
  if (*this == Newton()) return "N";
  if (*this == Joule()) return "J";
  if (*this == Watt()) return "W";
  if (*this == MetersPerSecond()) return "m/s";
  static std::map<SiBaseUnit, std::string> units = {
      {SiBaseUnit::UNITLESS, ""},   {SiBaseUnit::AMPERE, "amp"},
      {SiBaseUnit::CANDELA, "can"}, {SiBaseUnit::KELVIN, "kelvin"},
      {SiBaseUnit::KILOGRAM, "kg"}, {SiBaseUnit::METER, "m"},
      {SiBaseUnit::MOLE, "mol"},    {SiBaseUnit::SECOND, "sec"}};
  char buf[200];
  std::string s = "";
  if (units_.size() > 1) s += "(";
  for (const auto& p : units_) {
    if (s != "" && s != "(") s += " ";
    s += units[p.first];
    if (p.second != 1) {
      sprintf(buf, "%d", p.second);
      s += buf;
    }
  }
  if (units_.size() > 1) s += ")";
  return s;
}

SiUnit SiUnit::Kilogram() { return SiBaseUnit::KILOGRAM; }
SiUnit SiUnit::Meter() { return SiBaseUnit::METER; }
SiUnit SiUnit::Second() { return SiBaseUnit::SECOND; }
SiUnit SiUnit::Newton() { return Kilogram() * Meter() * Second().Power(-2); }
SiUnit SiUnit::Joule() { return Newton() * Meter(); }
SiUnit SiUnit::Watt() { return Joule() / Second(); }
SiUnit SiUnit::MetersPerSecond() { return Meter() / Second(); }

}  //  namespace cycling
