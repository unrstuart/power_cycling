#ifndef __TIME_SAMPLE_H__
#define __TIME_SAMPLE_H__

#include <chrono>
#include <iostream>
#include <vector>

#include "measurement.h"
#include "si_var.h"

namespace cycling {

// Holds a collection of measurements that were all taken at a specific point in
// time. See Measurement::Type for details of what types of measurements are
// recorded.
class TimeSample {
 public:
  using TimePoint = std::chrono::system_clock::time_point;

  // An iterator to allow immutably iterating through all measurements in the
  // sample.
  class const_iterator;

  TimeSample() = default;

  TimeSample(const TimePoint& time);

  TimeSample(const TimePoint& time, const Measurement& measurement);

  TimeSample(const TimePoint& time,
             const std::vector<Measurement>& measurements);

  TimeSample(const TimeSample&) = default;
  TimeSample(TimeSample&&) = default;

  TimeSample& operator=(const TimeSample&) = default;
  TimeSample& operator=(TimeSample&&) = default;

  bool operator==(const TimeSample& rhs) const;
  bool operator!=(const TimeSample& rhs) const;
  bool operator<(const TimeSample& rhs) const;
  bool operator>(const TimeSample& rhs) const;
  bool operator<=(const TimeSample& rhs) const;
  bool operator>=(const TimeSample& rhs) const;

  friend std::ostream& operator<<(std::ostream& lhs, const TimeSample& rhs);

  // Adds the measurement either replacing the existing
  // measurement of the same type, or added to the TimeSample if a measurement
  // of the same type was not previously present.
  TimeSample& Add(const Measurement& m);

  const TimePoint& time() const { return time_; }
  void set_time(const TimePoint& t) { time_ = t; }

  bool has_value(const Measurement::Type type) const;

  SiVar value(const Measurement::Type type) const;

  void set(const Measurement::Type type, const SiVar value);

 private:
  TimePoint time_;
  bool has_[Measurement::NUM_MEASUREMENTS] = {false};
  SiVar measurements_[Measurement::NUM_MEASUREMENTS];
};

}  // namespace cycling

#endif  // __TIME_SAMPLE_H__
