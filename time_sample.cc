#include "time_sample.h"

#include <cassert>
#include <ctime>

namespace cycling {

TimeSample::TimeSample(const TimePoint& time) : time_(time) {}

TimeSample::TimeSample(const TimePoint& time, const Measurement& m)
    : time_(time) {
  has_[m.type()] = true;
  measurements_[m.type()] = m.value();
}

TimeSample::TimeSample(const TimePoint& time,
                       const std::vector<Measurement>& measurements)
    : time_(time) {
  for (const auto& m : measurements) Add(m);
}

TimeSample& TimeSample::Add(const Measurement& m) {
  has_[m.type()] = true;
  measurements_[m.type()] = m.value();
  return *this;
}

bool TimeSample::has_value(const Measurement::Type type) const {
  return has_[type];
}

SiVar TimeSample::value(const Measurement::Type type) const {
  assert(has_value(type));
  return measurements_[type];
}

bool TimeSample::operator==(const TimeSample& rhs) const {
  if (time_ != rhs.time_) return false;
  for (int i = 0; i < Measurement::NUM_MEASUREMENTS; ++i) {
    if (has_[i] != rhs.has_[i]) return false;
    if (has_[i] && measurements_[i] != rhs.measurements_[i]) return false;
  }
  return true;
}

bool TimeSample::operator!=(const TimeSample& rhs) const {
  return !(*this == rhs);
}

bool TimeSample::operator<(const TimeSample& rhs) const {
  if (time_ != rhs.time_) return time_ < rhs.time_;
  for (int i = 0; i < Measurement::NUM_MEASUREMENTS; ++i) {
    if (has_[i] != rhs.has_[i]) return !has_[i];
    if (has_[i] && measurements_[i] != rhs.measurements_[i]) {
      return measurements_[i] < rhs.measurements_[i];
    }
  }
  return false;
}

bool TimeSample::operator>(const TimeSample& rhs) const {
  if (time_ != rhs.time_) return time_ > rhs.time_;
  return measurements_ > rhs.measurements_;
}

bool TimeSample::operator<=(const TimeSample& rhs) const {
  return !(*this > rhs);
}

bool TimeSample::operator>=(const TimeSample& rhs) const {
  return !(*this < rhs);
}

std::ostream& operator<<(std::ostream& lhs, const TimeSample& rhs) {
  const std::time_t time = std::chrono::system_clock::to_time_t(rhs.time());
  lhs << std::ctime(&time) << " {";
  for (int i = 0; i < Measurement::NUM_MEASUREMENTS; ++i) {
    const Measurement::Type m = static_cast<Measurement::Type>(i);
    if (!rhs.has_value(m)) continue;
    lhs << ' ' << rhs.value(m);
  }
  return lhs << " }";
}

}  // namespace cycling
