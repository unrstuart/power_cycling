#ifndef __TCX_UTIL_H__
#define __TCX_UTIL_H__

#include <memory>
#include <string>

#include "time_series.h"

namespace cycling {

// Reads the TCX file at the given path and converts it to a time series.
// Returns null if the file was bad.
std::unique_ptr<TimeSeries> ConvertTcxToTimeSeries(const std::string& file);

}  // namespace cycling

#endif
