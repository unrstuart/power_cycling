#ifndef __GRAPHER_H__
#define __GRAPHER_H__

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
  using Duration = std::chrono::system_clock::duration;
  using TimePoint = TimeSample::TimePoint;

  // A single point to be graphed in logical space (x and y in [0,1]).
  struct Point {
    double x, y;
  };

  // A label on the graph with a y-axis location and an integral value.
  struct Label {
    double y;
    int64_t value;
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
    std::vector<Label> labels;
    // The points to be plotted. first=x, second=y.
    std::vector<Point> points;
  };

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
  static Graph Plot(const TimeSeries& series, const TimePoint& current_time,
                    const Duration& width, const Duration& increment,
                    const Duration& look_behind, const Measurement::Type type,
                    const double coef, const double stage);
};

}  // namespace cycling

#endif  // __GRAPHER_H__
