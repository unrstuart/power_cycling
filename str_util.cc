#include "str_util.h"

#include <cctype>

namespace cycling {

std::string TrimWhitespace(const std::string& str) {
  if (str.empty() || (*str.begin() > ' ' && *str.rbegin() > ' ')) return str;
  auto start = str.begin();
  for (; start != str.end() && *start <= ' '; ++start) {
  }
  auto end = str.begin() + str.size() - 1;
  for (; *end <= ' '; --end) {
  }
  return str.substr(start - str.begin(), end - start + 1);
}

std::string ToLowercase(const std::string& str) {
  std::string ret = str;
  for (char& c : ret) c = tolower(c);
  return ret;
}

}  // namespace cycling
