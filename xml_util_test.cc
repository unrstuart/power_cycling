#include "xml_util.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace cycling {
namespace xml_util {
namespace {

using ::testing::AllOf;
using ::testing::Not;

const char kTestData[] = R"(
  <a>
    <b first="">
      <c first=""/>
      <d first=""/>
    </b>
    <b second="">
      <c second=""/>
      <d second=""/>
    </b>
  </a>
)";

const char kTestFilePath[] = "xml_util_test_data.xml";

MATCHER_P(HasName, name,
          "is " + std::string(negation ? "not " : "") + "named " + name) {
  return arg != nullptr && arg->type != XmlNode::FREE_TEXT && arg->name == name;
}

MATCHER_P2(HasAttribute, attr_name, attr_value,
           std::string(negation ? "has " : "does not have ") +
               "the attribute " + std::string(attr_name) +
               (std::string(attr_value).empty()
                    ? std::string()
                    : "=" + std::string(attr_value))) {
  if (arg == nullptr) return false;
  if (arg->type == XmlNode::FREE_TEXT) return false;
  for (const auto& attr : arg->attrs) {
    if (attr.first == attr_name && attr.second == attr_value) return true;
  }
  return false;
}

bool XmlTreesAreEqual(const XmlNode* a, const XmlNode* b) {
  if (a == nullptr || b == nullptr) return a == b;
  if (a->type != b->type) return false;
  if (a->type == XmlNode::FREE_TEXT) return a->text == b->text;
  if (a->name != b->name) return false;
  if (a->attrs.size() != b->attrs.size()) return false;
  if (a->children.size() != b->children.size()) return false;
  for (const auto& attr : a->attrs) {
    auto it = b->attrs.find(attr.first);
    if (it == b->attrs.end() && attr.second != it->second) return false;
  }
  for (auto it = a->children.begin(); it != a->children.end(); ++it) {
    auto it2 = b->children.begin() + (it - a->children.begin());
    if (!XmlTreesAreEqual(it->get(), it2->get())) return false;
  }
  return true;
}

class XmlUtilTest : public ::testing::Test {
 protected:
  void SetUp() {
    root_ = ParseXmlContents(kTestData);
    ASSERT_NE(root_, nullptr);
  }

  bool NodeHasName(const XmlNode* node, const std::string& name) const {
    return node != nullptr && node->type != XmlNode::FREE_TEXT &&
           node->name == name;
  }

  void PrintTree(const XmlNode* node, const int indent) {
    if (node == nullptr) return;
    printf("%*s", indent * 2, "");
    switch (node->type) {
      case XmlNode::FREE_TEXT:
        printf("%s\n", node->text.c_str());
        break;
      case XmlNode::ROOT:
      case XmlNode::TAG:
        printf("%s", node->name.c_str());
        for (const auto& attr : node->attrs) {
          printf(" %s='%s'", attr.first.c_str(), attr.second.c_str());
        }
        printf("\n");
        for (const auto& kid : node->children) {
          PrintTree(kid.get(), indent + 1);
        }
        break;
    }
  }

  std::unique_ptr<XmlNode> root_;
};

TEST_F(XmlUtilTest, Parse) {
  ASSERT_TRUE(XmlTreesAreEqual(root_.get(), ParseXmlFile(kTestFilePath).get()));
}

TEST_F(XmlUtilTest, FindNode) {
  PrintTree(root_.get(), 0);
  EXPECT_THAT(FindNode("a", root_.get()), HasName("a"));
  EXPECT_THAT(FindNode("b", root_.get()), HasName("b"));
  EXPECT_EQ(FindNode("e", root_.get()), nullptr);
}

TEST_F(XmlUtilTest, FindNextNodeFails) {
  EXPECT_EQ(FindNextNode("a", root_.get()), nullptr);
  EXPECT_EQ(FindNextNode("b", root_->children[1].get()), nullptr);
}

TEST_F(XmlUtilTest, FindNextNodeInChild) {
  EXPECT_THAT(FindNextNode("b", root_.get()),
              AllOf(HasName("b"), HasAttribute("first", "")));
}

TEST_F(XmlUtilTest, FindNextNodeInSibling) {
  const XmlNode* b = FindNode("b", root_.get());
  ASSERT_THAT(b, AllOf(HasName("b"), HasAttribute("first", "")));
  EXPECT_THAT(FindNextNode("b", b),
              AllOf(HasName("b"), HasAttribute("second", "")));
}

TEST_F(XmlUtilTest, FindNextNodeInUncle) {
  const XmlNode* c = FindNode("c", root_.get());
  ASSERT_THAT(c, AllOf(HasName("c"), HasAttribute("first", "")));
  EXPECT_THAT(FindNextNode("b", c),
              AllOf(HasName("b"), HasAttribute("second", "")));
}

TEST_F(XmlUtilTest, FindNextNodeInCousin) {
  const XmlNode* c = FindNode("c", root_.get());
  ASSERT_THAT(c, AllOf(HasName("c"), HasAttribute("first", "")));
  EXPECT_THAT(FindNextNode("c", c),
              AllOf(HasName("c"), HasAttribute("second", "")));
}

}  // namespace
}  // namespace xml_util
}  // namespace cycling
