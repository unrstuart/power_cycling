#ifndef __XML_UTIL_H__
#define __XML_UTIL_H__

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace cycling {

// Contains either free text in an XML document, or a tag complete with
// attributes and child nodes representing free text and sub tags.
struct XmlNode {
  enum Type {
    ROOT,
    FREE_TEXT,
    TAG,
  };
  Type type;
  std::string text; // Only set if this is a text node.

  // Only set if this is a root or tag node.
  std::vector<std::unique_ptr<XmlNode>> children;

  // All of these are only set if this is a tag node.
  std::string name;
  std::map<std::string, std::string> attrs;
};

std::unique_ptr<XmlNode> ParseXmlFile(const std::string& path);

}  // namespace cycling

#endif

