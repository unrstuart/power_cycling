#include "time_sample.h"

#include <cassert>
#include <ctime>

namespace cycling {

TimeSample::TimeSample(const TimePoint& time) : time_(time) {}

TimeSample::TimeSample(const TimePoint& time, const Measurement& m)
    : time_(time) {
  measurements_[m.type()] = m.value();
}

TimeSample::TimeSample(const TimePoint& time,
                       const std::vector<Measurement>& measurements)
    : time_(time) {
  for (const auto& m : measurements) {
    measurements_[m.type()] = m.value();
  }
}

TimeSample TimeSample::Add(const Measurement& m) const {
  TimeSample t = *this;
  t.measurements_[m.type()] = m.value();
  return t;
}

bool TimeSample::has_value(const Measurement::Type type) const {
  return measurements_.find(type) != measurements_.end();
}

SiVar TimeSample::value(const Measurement::Type type) const {
  auto it = measurements_.find(type);
  assert(it != measurements_.end());
  return it->second;
}

bool TimeSample::operator==(const TimeSample& rhs) const {
  return time_ == rhs.time_ && measurements_ == rhs.measurements_;
}

bool TimeSample::operator!=(const TimeSample& rhs) const {
  return !(*this == rhs);
}

bool TimeSample::operator<(const TimeSample& rhs) const {
  if (time_ != rhs.time_) return time_ < rhs.time_;
  return measurements_ < rhs.measurements_;
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
  for (const auto& p : rhs) {
    lhs << ' ' << p.second.ToString();
  }
  return lhs;
}

TimeSample::const_iterator TimeSample::begin() const {
  return measurements_.begin();
}

TimeSample::const_iterator TimeSample::end() const {
  return measurements_.end();
}

}  // namespace cycling
