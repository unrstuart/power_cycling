#ifndef __SI_BASE_UNIT_H__
#define __SI_BASE_UNIT_H__

#include <string>

namespace cycling {

// The seven basic units of SI measurements. A unitless value is also included
// to mark coefficients and units that have been multiplied by zero.
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
