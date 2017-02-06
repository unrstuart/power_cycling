#include <chrono>
#include <map>
#include <vector>

#include "measurement.h"
#include "time_series.h"

namespace cycling {

// Holds state and functionality needed to graph one measurement from a time
// series.
//
// The graph scales dynamically with the displayed content, and this
// class has support to smoothly transition from one graph size to the next (if
// in the appropriate mode). See Plot() for details.
//
// The dimensions of the graph in the metric's space are only affected by what
// is in the current window and, if transitioning, the next window, including
// look behind.
//
// This class is thread safe.
class Grapher {
 public:
  using Duration = std::chrono::duration;
  using TimePoint = TimeSample::TimePoint;

  // A single point to be graphed in logical space (x and y in [0,1]).
  struct Point {
    double x, y;
  };

  // The result of a call to Plot.
  struct Graph {
    // The minimum time value plotted. Not necessarily the value of any of the
    // points plotted.
    TimePoint min_x;
    // The maximum time value plotted. Not necessarily the value of any of the
    // points plotted.
    TimePoint max_x;
    // The minimum y value displayed.
    double min_y;
    // The maximum y value displayed.
    double max_y;
    // Labels of coefficients that can be displayed on the y-axis.
    std::map<double, int> labels;
    // The points to be plotted. first=x, second=y.
    std::vector<Point> points;
  };

  // Creates a new grapher using the given time-window parameters. No tranfer of
  // ownership.
  Grapher(const TimePoint& width, const TimePoint& increment,
          const TimePoint& look_behind);
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
  Graph Plot(const TimeSeries& series, const TimePoint& current_time,
             const Measurement::Type type, const double coef,
             const double stage) const;

 private:
  const TimePoint width_;
  const TimePoint increment_;
  const TimePoint look_behind_;
};

}  // namespace cycling
