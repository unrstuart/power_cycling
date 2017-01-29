#include "si_base_unit.h"

#include "gtest/gtest.h"

namespace cycling {
namespace {

TEST(SiBaseUnitTest, ToString) {
  EXPECT_FALSE(ToString(SiBaseUnit::UNITLESS).empty());
}

}  // namespace
}  // namespace cycling
