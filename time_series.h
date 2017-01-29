#ifndef __TIME_SERIES_H__
#define __TIME_SERIES_H__

#include <functional>
#include <map>

#include "time_sample.h"

namespace cycling {

// Holds a collection of sequential, but not necessarily uniformly separated,
// TimeSamples.
class TimeSeries {
 public:
  using TimePoint = TimeSample::TimePoint;
  using SampleVisitor = std::function<void(const TimeSample&)>;

  TimeSeries() {}
  TimeSeries(const TimeSeries&) = delete;
  TimeSeries(TimeSeries&& rhs) = delete;
  ~TimeSeries() = default;

  TimeSeries& operator=(const TimeSeries&) = delete;
  TimeSeries& operator=(TimeSeries&& rhs) = delete;

  // sample must come strictly later than the last time sample currently
  // contained.
  void Add(TimeSample&& sample);
  TimePoint BeginTime() const;
  TimePoint EndTime() const;

  void Visit(const TimePoint& begin, const TimePoint& end,
             const SampleVisitor& visitor) const;

 private:
  std::vector<TimeSample> samples_;
};

}  // namespace cycling

#endif
