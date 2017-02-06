#include "grapher.h"

#include <algorithm>

#include "measurement.h"
#include "time_series.h"

namespace cycling {

Grapher::Grapher(const TimePoint& start, const TimePoint& width,
                 const TimePoint& increment, const TimePoint& look_behind)
    : width_(width), increment_(increment), look_behind_(look_behind) {}

Grapher::Graph Grapher::Draw(const TimeSeries& series, const TimePoint& start,
                             const Measurement::Type type, const double coef,
                             const double stage) const {
  Graph graph = {start, start + window, 0, 1};

  series.Visit(start - look_behind_ + increment * stage,
               start + window + increment * stage, type, store_data);

  return graph;
}

}  // namespace cycling
