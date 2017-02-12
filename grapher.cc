#include "grapher.h"

#include <cmath>
#include <cstdlib>

#include <algorithm>
#include <map>

#include "measurement.h"
#include "time_series.h"

namespace cycling {

namespace {

struct Sample {
  using TimePoint = Grapher::TimePoint;
  TimePoint time;
  double data;
  bool operator<(const Sample& rhs) const { return time < rhs.time; }
  bool operator==(const Sample& rhs) const { return time == rhs.time; }
  bool operator<(const TimePoint& rhs) const { return time < rhs; }
  bool operator==(const TimePoint& rhs) const { return time == rhs; }
};

std::vector<std::pair<double, int>> GetLabels(const double min,
                                              const double max,
                                              const int spacing) {
  std::vector<std::pair<double, int>> labels;
  for (int i = -1000; i <= max; i += spacing) {
    if (i >= min) labels.push_back({(i - min) / (max - min), i});
  }
  return labels;
}

// Returns the labels along the y-axis that should be used to display the graph
// between min and max. Stores the values of the min and max values displayed in
// used_min and used_max respectively.
std::vector<std::pair<double, int>> GetLabels(const double min,
                                              const double max,
                                              double* used_min,
                                              double* used_max) {
  if (min == max) {
    return {{min - 5, 0}, {max + 5, 1}};
  }

  // Always show at least ten units.
  const int diff_exp =
      static_cast<int>(std::floor(std::log(max - min + 10) / std::log(10)));
  const int spacing = static_cast<int>(std::pow(10, diff_exp));

  *used_min = min - 5;
  *used_max = max + 5;

  std::vector<std::pair<double, int>> labels =
      GetLabels(*used_min, *used_max, spacing);
  if (labels.size() < 2 && spacing > 10) {
    labels = GetLabels(*used_min, *used_max, spacing / 10);
  }
  return labels;
}

void ComputeMinMax(const std::vector<Sample>& data,
                   const Grapher::TimePoint& begin,
                   const Grapher::TimePoint& end, double* min, double* max) {
  auto it = std::lower_bound(data.begin(), data.end(), begin);
  if (it == data.end() || it->time > end) {
    *min = *max = 0;
    return;
  }
  *min = *max = it->data;
  ++it;
  while (it != data.end() && it->time <= end) {
    *min = std::min(*min, it->data);
    *max = std::max(*max, it->data);
    ++it;
  }
}

}  // namespace

Grapher::Grapher(const Duration& width, const Duration& increment,
                 const Duration& look_behind)
    : width_(width), increment_(increment), look_behind_(look_behind) {}

Grapher::Graph Grapher::Plot(const TimeSeries& series, const TimePoint& start,
                             const Measurement::Type type, const double coef,
                             const double stage) const {
  Graph graph = {start, start + width_, 0, 1};
  std::vector<Sample> data;
  data.reserve((width_ + increment_ + look_behind_).count() / 1000000);

  const double bbox_min =
      start.time_since_epoch().count() + (stage * increment_).count();
  const double bbox_max = (start + width_).time_since_epoch().count() +
                          (stage * increment_).count();

  series.Visit(start - look_behind_, start + width_ + increment_, type,
               [&](const TimePoint& time, const double value) {
                 const double d = value * coef;
                 data.push_back({time, d});
               });

  double min0, min1, max0, max1;
  ComputeMinMax(data, start, start + width_, &min0, &max0);
  ComputeMinMax(data, start + increment_, start + increment_ + width_, &min1,
                &max1);

  double min2 = min0 + (min1 - min0) * stage;
  double max2 = max0 + (max1 - max0) * stage;
  double used_min, used_max;
  graph.labels = GetLabels(min2, max2, &used_min, &used_max);
  graph.min_y = min2;
  graph.max_y = max2;

  for (const Sample& sample : data) {
    const double time = sample.time.time_since_epoch().count();
    graph.points.push_back({(time - bbox_min) / (bbox_max - bbox_min),
                            (sample.data - used_min) / (used_max - used_min)});
  }

  return graph;
}

}  // namespace cycling
