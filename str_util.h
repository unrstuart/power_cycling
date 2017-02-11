#ifndef __STR_UTIL_H__
#define __STR_UTIL_H__

#include <sstream>
#include <string>

namespace cycling {

std::string TrimWhitespace(const std::string& str);
std::string ToLowercase(const std::string& str);

template <typename Param>
std::string StrCat(const Param& param) {
  std::ostringstream stream;
  stream << param;
  return stream.str();
}

template <typename Param, typename... Params>
std::string StrCat(const Param& param, Params... params) {
  return StrCat(param) + StrCat(params...);
}

}  // namespace

#endif
