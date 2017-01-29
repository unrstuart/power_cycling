#include "time_series.h"

#include <cassert>

#include <algorithm>

namespace cycling {

void TimeSeries::Add(TimeSample&& sample) {
  assert(samples_.empty() || sample.time() > samples_.back().time());
  samples_.push_back(sample);
}

TimeSeries::TimePoint TimeSeries::BeginTime() const {
  assert(!samples_.empty());
  return samples_.front().time();
}

TimeSeries::TimePoint TimeSeries::EndTime() const {
  assert(!samples_.empty());
  return samples_.back().time();
}

void TimeSeries::Visit(const TimePoint& begin, const TimePoint& end,
                       const SampleVisitor& visitor) const {
  auto b = std::lower_bound(samples_.begin(), samples_.end(), begin);
  --b;
  if (b == samples_.end()) b = samples_.begin();
  auto e = std::lower_bound(b, samples_.end(), end);
  while (b != e) {
    visitor(*b);
    ++b;
  }
}

}  // namespace cycling
