#ifndef __MEASUREMENT_H__
#define __MEASUREMENT_H__

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
    // The total calories burned thus far, up to and including this sample.
    TOTAL_CALORIES,
  };

  Measurement(const Type type, const SiVar value)
      : type_(type), value_(value) {}

  std::string ToString() const;

  Type type() const { return type_; }
  SiVar value() const { return value_; }

 private:
  Type type_;
  SiVar value_;
};

}  // namespace cycling

#endif  // __MEASUREMENT_H__
