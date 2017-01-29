#include "time_sample.h"

#include <cassert>

namespace cycling {

TimeSample::TimeSample(const TimePoint& time) : time_(time) {}

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

SiVar TimeSample::value(const Measurement::Type type) const {
  auto it = measurements_.find(type);
  assert(it != measurements_.end());
  return it->second;
}

}  // namespace cycling
