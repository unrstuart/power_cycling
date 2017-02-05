#ifndef __MEASUREMENT_H__
#define __MEASUREMENT_H__

#include <iostream>
#include <string>

#include "si_var.h"

namespace cycling {

// Holds one sample of one type of measurement. E.g. HR=143 BPM.
class Measurement {
 public:
  enum Type {
    NO_TYPE,
    // Heart rate, in beats per minute. Unitless.
    HEART_RATE,
    // The time between the end of one beat and the beginning of the next.
    HRV,
    // Speed
    SPEED,
    // Total cadence (as opposed to per leg). Unitless.
    CADENCE,
    // Instantenous power.
    POWER,
    // Bike gear. Unitless.
    GEAR,
    // The total distance covered up to, and including, this sample.
    TOTAL_DISTANCE,
    // The total joules burned thus far, up to and including this sample.
    TOTAL_JOULES,
  };

  // Constructs a new measurement with the appropriate units. coef is for the
  // base unit of the measurement type. E.g. for SPEED, coef is interpreted as
  // m/s, not km/h.
  Measurement(const Type type, const double coef);

  // Same as above, but uses the units supplied in var. asserts that the units
  // are correct.
  Measurement(const Type type, const SiVar& var);
  Measurement(const Measurement&) = default;
  Measurement(Measurement&&) = default;
  Measurement& operator=(const Measurement&) = default;
  Measurement& operator=(Measurement&&) = default;

  bool operator==(const Measurement& rhs) const;
  bool operator!=(const Measurement& rhs) const;
  bool operator<(const Measurement& rhs) const;
  bool operator>(const Measurement& rhs) const;
  bool operator<=(const Measurement& rhs) const;
  bool operator>=(const Measurement& rhs) const;

  friend std::ostream& operator<<(std::ostream& lhs, const Measurement& rhs) {
    return lhs << rhs.ToString();
  }

  std::string ToString() const;

  Type type() const { return type_; }
  SiVar value() const { return value_; }

 private:
  Type type_;
  SiVar value_;
};

}  // namespace cycling

#endif  // __MEASUREMENT_H__
