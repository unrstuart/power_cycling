#include "str_util.h"
#include "gtest/gtest.h"

namespace cycling {
namespace {

TEST(StrUtilTest, Whitespace) {
  EXPECT_EQ(TrimWhitespace(""), "");
  EXPECT_EQ(TrimWhitespace("\n"), "");
  EXPECT_EQ(TrimWhitespace(" \n \n \r\t  "), "");
  EXPECT_EQ(TrimWhitespace("a"), "a");
  EXPECT_EQ(TrimWhitespace("ab"), "ab");
  EXPECT_EQ(TrimWhitespace(" ab"), "ab");
  EXPECT_EQ(TrimWhitespace("ab "), "ab");
  EXPECT_EQ(TrimWhitespace(" ab "), "ab");
  EXPECT_EQ(TrimWhitespace(" a b "), "a b");
}

TEST(StrUtilTest, ToLowercase) {
  EXPECT_EQ(ToLowercase("abc"), "abc");
  EXPECT_EQ(ToLowercase("aBc"), "abc");
  EXPECT_EQ(ToLowercase("AbC"), "abc");
  EXPECT_EQ(ToLowercase("ABC"), "abc");
}

}  // namespace
}  // namespace cycling
