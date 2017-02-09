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

  // The parent node of this node. Might be null.
  XmlNode* parent = nullptr;

  // Only set if this is a text node.
  std::string text;

  // Only set if this is a root or tag node.
  std::vector<std::unique_ptr<XmlNode>> children;

  // All of these are only set if this is a tag node.
  std::string name;
  std::map<std::string, std::string> attrs;
};

// Parses the XML file at the given path and converts it to a tree of XmlNodes.
std::unique_ptr<XmlNode> ParseXmlFile(const std::string& path);

// Does a DFS from root for the first node whose name matches.
const XmlNode* FindNode(const std::string& name, const XmlNode* root);

// Does a smarter DFS from the root for next node whose name matches. This
// function actually starts crawling below hint. If it can't find anything, it
// will then move on to any sibling of hint that comes after it in its parent's
// children. The same procedure is followed recursively up the tree.
const XmlNode* FindNextNode(const std::string& name, const XmlNode* hint);

}  // namespace cycling

#endif
