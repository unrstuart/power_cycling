#ifndef __TCX_UTIL_H__
#define __TCX_UTIL_H__

#include <string>

#include "time_series.h"

namespace cycling {

// Converts the TCX file at path to a TimeSeries, stripping out lap information.
std::unique_ptr<TimeSeries> ParseTcxFile(const std::string& path);

}  // namespace cycling

#endif
