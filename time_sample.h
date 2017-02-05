#ifndef __TIME_SAMPLE_H__
#define __TIME_SAMPLE_H__

#include <chrono>
#include <iostream>
#include <map>
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

  TimeSample(const TimePoint& time, const Measurement& measurements);

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

  // Returns a new TimeSample with the measurement either replacing the existing
  // measurement of the same type, or added to the TimeSample if a measurement
  // of the same type was not previously present.
  TimeSample Add(const Measurement& m) const;

  const TimePoint& time() const { return time_; }

  bool has_value(const Measurement::Type type) const;

  SiVar value(const Measurement::Type type) const;

  void set(const Measurement::Type type, const SiVar value);

  const_iterator begin() const;
  const_iterator end() const;

 private:
  TimePoint time_;
  std::map<Measurement::Type, SiVar> measurements_;
};

class TimeSample::const_iterator {
 public:
  using base_map = std::map<Measurement::Type, SiVar>;
  using base_iterator = base_map::const_iterator;
  
  const_iterator(const base_iterator& it) : it_(it) {}
  const_iterator(const const_iterator& it) : it_(it.it_) {}
  const_iterator(const_iterator&& it) : it_(it.it_) {}

  bool operator==(const const_iterator& rhs) const { return it_ == rhs.it_; }
  bool operator!=(const const_iterator& rhs) const { return it_ != rhs.it_; }
  const_iterator& operator++() {
    ++it_;
    return *this;
  }
  const_iterator& operator--() {
    --it_;
    return *this;
  }
  const base_map::value_type& operator*() const { return *it_; }
  const base_map::value_type* operator->() const { return it_.operator->(); }

 private:
  base_iterator it_;
};

}  // namespace cycling

#endif  // __TIME_SAMPLE_H__
