#include <chrono>
#include <map>

#include "measurement.h"
#include "time_series.h"

namespace cycling {

// Holds state and functionality needed to graph one measurement from a time
// series.
//
// The graph scales dynamically with the displayed content, and this
// class has support to smoothly transition from one graph size to the next (if
// in the appropriate mode). See Draw() for details.
//
// The dimensions of the graph in the metric's space are only affected by what
// is in the current window and, if transitioning, the next window. Anything
// behind the current data is drawn, but might be clipped.
//
// This class is thread safe.
class Grapher {
 public:
  using Duration = std::chrono::duration;
  using TimePoint = TimeSample::TimePoint;

  // Creates a new grapher using the given time-window parameters. No tranfer of
  // ownership.
  Grapher(const TimePoint& start, const TimePoint& width,
          const TimePoint& increment, const TimePoint& look_behind);
  ~Grapher() = default;

  // stage is clamped to [0,1]. At 0, this object graphs with the contents
  // locked in the given window. At 1, this object graphs with the contents
  // locked in the window advanced by one unit.
  //
  // Given the scale and bounds of the numbers being graphed, this class
  // attempts to draw horizontal lines as appropriate. The vertical locations
  // and the labels are set in labels (if not null). The labels aren't actually
  // drawn, they are merely provided as a convenience so the user may draw the
  // labels, if they so choose.
  //
  // All values are pre-multiplied by coef, which comes in handy when the values
  // being graphed have units different from their SI representative (e.g. one
  // wishes to graph km/h instead of m/s).
  //
  // The output graph is in the space x=[0,1],y=[0,1]. Any scaling must be done
  // outside by callers of this function.
  void Draw(const TimeSeries& series, const TimePoint& current_time,
            const Measurement::Type type, const double coef, const double stage,
            std::map<double, double>* labels) const;

 private:
  const TimePoint start_;
  const TimePoint width_;
  const TimePoint increment_;
  const TimePoint look_behind_;
};

}  // namespace cycling
