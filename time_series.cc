#include "time_series.h"

#include <cassert>

#include <algorithm>

namespace cycling {

void TimeSeries::Add(const TimeSample& sample) {
  TimeSample s(sample);
  Add(std::move(s));
}

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
                       const Measurement::Type type,
                       const MeasurementVisitor& visitor) const {
  auto b = std::lower_bound(samples_.begin(), samples_.end(), begin);
  if (b == samples_.end()) b = samples_.begin();
  while (b != samples_.end() && b->time() <= end) {
    if (b->has_value(type)) visitor(b->time(), b->value(type).coef());
    ++b;
  }
}

void TimeSeries::Visit(const TimePoint& begin, const TimePoint& end,
                       const SampleVisitor& visitor) const {
  auto b = std::lower_bound(samples_.begin(), samples_.end(), begin);
  if (b == samples_.end()) b = samples_.begin();
  while (b != samples_.end() && b->time() <= end) {
    visitor(*b);
    ++b;
  }
}

}  // namespace cycling
