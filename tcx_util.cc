#include "tcx_util.h"

#include <map>
#include <memory>
#include <string>

#include "measurement.h"
#include "si_base_unit.h"
#include "si_unit.h"
#include "si_var.h"
#include "time_sample.h"
#include "time_series.h"
#include "xml_util.h"

namespace cycling {

namespace {

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

std::map<std::string, int> names;

void BeginParse(const XmlNode* node, TimeSeries* series) {
  if (node == nullptr) return;
  switch (node->type) {
    case XmlNode::ROOT:
    case XmlNode::TAG:
      names[node->name]++;
      for (const auto& kid : node->children) {
        BeginParse(kid.get(), series);
      }
      break;
    case XmlNode::FREE_TEXT:
      names["(free text)"]++;
      return;
  }
}

}  // namespace

std::unique_ptr<TimeSeries> ParseTcxFile(const std::string& path) {
  std::unique_ptr<XmlNode> node = xml_util::ParseXmlFile(path);
  if (!node) return nullptr;
  TimeSeries series;
  BeginParse(node.get(), &series);
  return make_unique<TimeSeries>(std::move(series));
}

}  // namespace cycling
