#include "grapher.h"

#include "measurement.h"
#include "time_series.h"

namespace cycling {

Grapher::Grapher(const Window& window, const TimePoint& start_time)
    : window_(window), current_time_(start_time) {}

bool Grapher::MoveToNext() { current_time_ += window_.increment; }

void Draw(const TimeSeries& series, const Measurement::Type type,
          const double stage) const {
  double max
  map<> look_behind_data;
  
}

}  // namespace cycling
