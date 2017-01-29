#include "si_unit.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using gtest::ElementsAre;
using gtest::Pair;

namespace cycling {
namespace {

TEST(SiUnitTest, Ctor) {
  SiUnit u0;
  SiUnit u1 = SiUnit::Kilogram();
  SiUnit u2 = u1;
  SiUnit u3(u2);
  EXPECT_EQ(u0, SiUnit::Unitless());
  EXPECT_EQ(u1, SiUnit::Kilogram());
  EXPECT_THAT(u1, ElementsAre(Pair(SiBaseUnit::KILOGRAM, 1)));
}

}  // namespace
}  // namespace cycling
