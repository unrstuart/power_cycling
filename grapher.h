#include <chrono>

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
// Simultaneous calls to Draw() are thread safe, however
class Grapher {
 public:
  using Duration = std::chrono::duration;
  using TimePoint = TimeSample::TimePoint;

  struct Window {
    // The time at which to start graphing.
    const TimePoint start;
    // The amount of time to display.
    const Duration width;
    // By how much the graph unit should slide with each input.
    const Duration increment;
    // How far behind the "width" we should graph.
    const Duration look_behind;
  };

  // Creates a new grapher using the given time-window parameters. No tranfer of
  // ownership.
  Grapher(const Window& window, const TimePoint& start_time);
  ~Grapher() = default;

  // Moves the view window over by one unit.
  bool MoveToNext();

  // stage is clamped to [0,1]. At 0, this object graphs with the contents
  // locked in the given window. At 1, this object graphs with the contents
  // locked in the window advanced by one unit.
  //
  // The output graph is in the space x=[0,1],y=[0,1]. Any scaling must be done
  // outside by callers of this function.
  void Draw(const TimeSeries& series, const Measurement::Type type,
            const double stage) const;

 private:
  Window window_;
  TimePoint current_time_;
};

}  // namespace cycling
