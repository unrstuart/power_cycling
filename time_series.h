#ifndef __TIME_SERIES_H__
#define __TIME_SERIES_H__

#include <functional>

#include "time_sample.h"

namespace cycling {

// Holds a collection of sequential, but not necessarily uniformly separated,
// TimeSamples.
class TimeSeries {
 public:
  using TimePoint = TimeSample::TimePoint;
  using MeasurementVisitor =
      std::function<void(const TimePoint&, const double)>;
  using SampleVisitor =
      std::function<void(const TimeSample&)>;

  TimeSeries() = default;
  TimeSeries(const TimeSeries&) = delete;
  TimeSeries(TimeSeries&& rhs) = default;
  ~TimeSeries() = default;
  TimeSeries& operator=(const TimeSeries&) = delete;
  TimeSeries& operator=(TimeSeries&& rhs) = default;

  // sample must come strictly later than the last time sample currently
  // contained.
  void Add(const TimeSample& sample);
  void Add(TimeSample&& sample);
  TimePoint BeginTime() const;
  TimePoint EndTime() const;
  int num_samples() const { return samples_.size(); }

  // Calls visitor for every measurement of type `type` in the range
  // [begin,end).
  void Visit(const TimePoint& begin, const TimePoint& end,
             const Measurement::Type type,
             const MeasurementVisitor& visitor) const;

  // Calls visitor for every measurement in the range [begin,end).
  void Visit(const TimePoint& begin, const TimePoint& end,
             const SampleVisitor& visitor) const;

 private:
  std::vector<TimeSample> samples_;
};

}  // namespace cycling

#endif
