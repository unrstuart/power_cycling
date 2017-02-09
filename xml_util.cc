#include "xml_util.h"

#include <cstdio>

#include <algorithm>
#include <utility>

#include "libxml/parser.h"
#include "libxml/tree.h"

namespace cycling {

namespace {

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

std::string ToString(const xmlChar* stupid_rep) {
  return stupid_rep ? std::string(reinterpret_cast<const char*>(stupid_rep))
                    : "";
}

std::map<xmlElementType, int> g_counts;
const std::map<xmlElementType, std::string> kTypes = {
    {XML_ELEMENT_NODE, "XML_ELEMENT_NODE"},
    {XML_ATTRIBUTE_NODE, "XML_ATTRIBUTE_NODE"},
    {XML_TEXT_NODE, "XML_TEXT_NODE"},
    {XML_CDATA_SECTION_NODE, "XML_CDATA_SECTION_NODE"},
    {XML_ENTITY_REF_NODE, "XML_ENTITY_REF_NODE"},
    {XML_ENTITY_NODE, "XML_ENTITY_NODE"},
    {XML_PI_NODE, "XML_PI_NODE"},
    {XML_COMMENT_NODE, "XML_COMMENT_NODE"},
    {XML_DOCUMENT_NODE, "XML_DOCUMENT_NODE"},
    {XML_DOCUMENT_TYPE_NODE, "XML_DOCUMENT_TYPE_NODE"},
    {XML_DOCUMENT_FRAG_NODE, "XML_DOCUMENT_FRAG_NODE"},
    {XML_NOTATION_NODE, "XML_NOTATION_NODE"},
    {XML_HTML_DOCUMENT_NODE, "XML_HTML_DOCUMENT_NODE"},
    {XML_DTD_NODE, "XML_DTD_NODE"},
    {XML_ELEMENT_DECL, "XML_ELEMENT_DECL"},
    {XML_ATTRIBUTE_DECL, "XML_ATTRIBUTE_DECL"},
    {XML_ENTITY_DECL, "XML_ENTITY_DECL"},
    {XML_NAMESPACE_DECL, "XML_NAMESPACE_DECL"},
    {XML_XINCLUDE_START, "XML_XINCLUDE_START"},
    {XML_XINCLUDE_END, "XML_XINCLUDE_END"},
    {XML_DOCB_DOCUMENT_NODE, "XML_DOCB_DOCUMENT_NODE"},
};

std::string TrimWhitespace(const std::string str) {
  int first_non_ws = 0;
  while (first_non_ws < str.size() && str[first_non_ws] <= ' ') ++first_non_ws;
  std::string ret = str.substr(first_non_ws);
  first_non_ws = ret.size() - 1;
  while (first_non_ws >= 0 && ret[first_non_ws] <= ' ') --first_non_ws;
  return ret.substr(0, first_non_ws + 1);
}

std::unique_ptr<XmlNode> ConvertToNode(xmlNode* cur_node) {
  auto node = make_unique<XmlNode>();
  g_counts[cur_node->type]++;
  switch (cur_node->type) {
    case XML_ELEMENT_NODE:
      node->name = ToString(cur_node->name);
      for (xmlAttr* attr = cur_node->properties; attr != nullptr;
           attr = attr->next) {
        xmlChar* value = xmlNodeListGetString(cur_node->doc, attr->children, 1);
        node->attrs[ToString(attr->name)] = ToString(value);
        xmlFree(value);
      }
      for (xmlNode* child = cur_node->children; child != nullptr;
           child = child->next) {
        node->children.push_back(ConvertToNode(child));
        node->children.back()->parent = node.get();
      }
      node->type = XmlNode::TAG;
      break;
    case XML_TEXT_NODE:
      node->text = TrimWhitespace(ToString(xmlNodeGetContent(cur_node)));
      node->type = XmlNode::FREE_TEXT;
      break;
    default:
      fprintf(stderr, "couldn't handle type %s\n",
              kTypes.find(cur_node->type)->second.c_str());
      exit(1);
  }
  return node;
}

std::unique_ptr<XmlNode> Trim(std::unique_ptr<XmlNode> node) {
  if (node == nullptr) return node;
  if (node->type == XmlNode::FREE_TEXT) {
    if (node->text.empty()) {
      return nullptr;
    }
    return node;
  }
  if (node->name.empty() && node->children.empty()) {
    return nullptr;
  }
  std::vector<std::unique_ptr<XmlNode>> new_kids;
  for (auto& p : node->children) {
    auto n = Trim(std::move(p));
    if (n) new_kids.push_back(std::move(n));
  }
  std::swap(node->children, new_kids);
  return node;
}

}  // namespace

std::unique_ptr<XmlNode> ParseXmlFile(const std::string& path) {
  xmlDoc* doc = NULL;
  xmlNode* root_element = NULL;

  doc = xmlReadFile(path.c_str(), NULL, 0);
  if (doc == NULL) return nullptr;

  root_element = xmlDocGetRootElement(doc);
  auto node = make_unique<XmlNode>();
  node->type = XmlNode::ROOT;

  for (xmlNode* child_node = root_element; child_node != nullptr;
       child_node = child_node->next) {
    node->children.push_back(ConvertToNode(child_node));
  }
  xmlFreeDoc(doc);
  xmlCleanupParser();

  return Trim(std::move(node));
}

const XmlNode* FindNode(const std::string& name, const XmlNode* root) {
  if (root == nullptr) return root;
  if (root->type == XmlNode::FREE_TEXT) return nullptr;
  if (root->name == name) return root;
  for (const auto& p : children) {
    const XmlNode* n = FindNode(name, p.get());
    if (n != nullptr) return n;
  }
  return nullptr;
}

const XmlNode* FindNextNode(const std::string& name, const XmlNode* parent, const XmlNode* hint) {
}

const XmlNode* FindNextNode(const std::string& name, const XmlNode* hint) {
  const XmlNode* n = FindNode(name, hint);
  if (n != nullptr) return n;
  return FindNextNode(name, hint->parent, hint);
}

}  // namespace cycling
