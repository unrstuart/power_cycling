#include "si_base_unit.h"
#include "si_base_unit.h"

namespace cycling {

std::string ToString(const SiBaseUnit base_unit) {
  switch (base_unit) {
    case SiBaseUnit::UNITLESS:
      return "unitless";
    case SiBaseUnit::AMPERE:
      return "ampere";
    case SiBaseUnit::CANDELA:
      return "candela";
    case SiBaseUnit::KELVIN:
      return "kelvin";
    case SiBaseUnit::KILOGRAM:
      return "kilogram";
    case SiBaseUnit::METER:
      return "meter";
    case SiBaseUnit::MOLE:
      return "mole";
    case SiBaseUnit::SECOND:
      return "second";
  }
}

}  // namespace cycling
