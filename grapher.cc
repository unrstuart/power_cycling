#include "grapher.h"

#include <algorithm>

#include "measurement.h"
#include "time_series.h"

namespace cycling {

Grapher::Grapher(const TimePoint& start, const TimePoint& width,
                 const TimePoint& increment, const TimePoint& look_behind)
    : start_(start),
      width_(width),
      increment_(increment),
      look_behind_(look_behind) {}

void Grapher::Draw(const TimeSeries& series, const Measurement::Type type,
                   const double coef, const double stage,
                   std::map<double, double>* labels) const {
  std::map<double, double> data;

  double min = -1, max = -1;

  const auto shift = increment_ * stage;
  const auto start = current_time + shift;
  const auto end = current_time + width_ + shift;
  const auto full_start = current_time - window_look_behind_;
  const double start_ticks = start.time_since_epoch().count();
  const double end_ticks = end.time_since_epoch().count();
  const double full_start_ticks = full_start.time_since_epoch().count();
  const double span = end_ticks - full_start_ticks;

  auto populate = [&](const TimePoint& time, const double val) {
    const double now_ticks = time.time_since_epoch().count();
    const double loc = (now_ticks - full_start_ticks) / span;
    const double scaled_val = val * coef;
    if (time >= start && time <= end) {
      if (min == -1) min = max = scaled_val;
      min = std::min(min, scaled_val);
      max = std::max(max, scaled_val);
    }
    data[loc] = val * coef;
  };

  ComputeLabels(min, max, labels);
  if (min == -1) {
    min = 0;
    max = 1;
    *labels[0] = 0;
    *labels[1] = 1;
  } else {
    // Figure out the OOM diff between min and max, then add labels for every
    // tenth of that between them.
    const double diff = max - min;
    const double diff_oom = std::floor(std::log(diff) / std::log(10));
    if (diff_oom == 0) {
    // The difference was less than ten.
    } else { 
    }
  }
}

}  // namespace cycling
