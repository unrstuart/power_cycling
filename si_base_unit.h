#ifndef __SI_BASE_UNIT_H__
#define __SI_BASE_UNIT_H__

#include <string>

namespace cycling {

enum class SiBaseUnit {
  UNITLESS,
  AMPERE,
  CANDELA,
  KELVIN,
  KILOGRAM,
  METER,
  MOLE,
  SECOND,
};

std::string ToString(const SiBaseUnit base_unit);

}  // namespace

#endif
