#include "grapher.h"

#include <cmath>

#include <algorithm>
#include <map>

#include "measurement.h"
#include "time_series.h"

namespace cycling {

namespace {

// Returns the labels along the y-axis that should be used to display the graph
// between min and max. Stores the values of the min and max values displayed in
// used_min and used_max respectively.
std::map<double, int> GetLabels(const double min, const double max,
                                double* used_min, double* used_max) {
  if (min == max && min == -1) {
    return {{0.0, 0}, {1.0, 1}};
  }
  const double shifted_max = std::min(max, min + 10);
  // Always show at least ten units.
  const int diff_exp =
      static_cast<int>(std::floor(std::log(shifted_max - min) / std::log(10)));
  const int spacing = static_cast<int>(std::pow(10, diff_exp));

  const double buffer_below_min =
      (min >= 0 ? std::fmod(min, spacing)
                : spacing - std::fmod(-imin, spacing));
  const double buffer_above_max =
      (shifted_max >= 0 ? spacing - std::fmod(imax, spacing)
                        : std::fmod(-imax, spacing));
  *used_min = min - buffer_below_min -
              (std::fmod(std::floor(min), spacing) == 0 ? spacing : 0);
  *used_max = shifted_max + buffer_above_max +
              (std::fmod(std::ceil(shifted_max), spacing) == 0 ? spacing : 0);

  for (int i = static_cast<int>(*used_min);
       i < static_cast<int>(*used_max) + spacing; i += spacing) {
    labels[(i - *used_min) / (*used_max - *used_min)] = i;
  }
}

}  // namespace

Grapher::Grapher(const TimePoint& start, const TimePoint& width,
                 const TimePoint& increment, const TimePoint& look_behind)
    : width_(width), increment_(increment), look_behind_(look_behind) {}

Grapher::Graph Grapher::Draw(const TimeSeries& series, const TimePoint& start,
                             const Measurement::Type type, const double coef,
                             const double stage) const {
  Graph graph = {start, start + window, 0, 1};
  std::map<TimePoint, double> data;

  const TimePoint bbox_min = start + std::floor(stage) * increment_;
  const TimePoint bbox_max = start + window_ + std::ceil(stage) * increment_;
  double min = -1, max = -1;

  series.Visit(start - look_behind_ + increment * stage,
               start + window + increment * stage, type,
               [&](const TimePoint& time, const double value) {
                 const double d = value * coef;
                 data[time] = d;
                 if (time >= bbox_min && time <= bbox_max) {
                   min = max = d;
                 } else {
                   min = std::min(min, d);
                   max = std::max(max, d);
                 }
               });

  double used_min, used_max;
  graph.labels = GetLabels(min, max, &used_min, &used_max);
  graph.min_y = min;
  graph.max_y = max;

  for (const auto& p : data) {
    graph->points.push_back(
        (p.first - bbox_min).count() /
            static_cast<double>((bbox_max - bbox_min).count()),
        (p.second - used_min) / (used_max - used_min));
  }

  return graph;
}

}  // namespace cycling
