#include "si_unit.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::ElementsAre;
using testing::Pair;
using testing::UnorderedElementsAre;

namespace cycling {
namespace {

TEST(SiUnitTest, Ctor) {
  SiUnit u0;
  SiUnit u1 = SiUnit::Kilogram();
  SiUnit u2 = u1;
  SiUnit u3(u2);
  EXPECT_EQ(u0, SiUnit::Unitless());
  EXPECT_EQ(u1, SiUnit::Kilogram());
  EXPECT_THAT(u1.units(), ElementsAre(Pair(SiBaseUnit::KILOGRAM, 1)));
  SiUnit u4({{SiBaseUnit::KILOGRAM, 1}});
  SiUnit u5(SiBaseUnit::KILOGRAM, 1);
  EXPECT_EQ(u4, u1);
  EXPECT_EQ(u1, u4);
  EXPECT_EQ(u4, u5);
  EXPECT_EQ(u5, u4);
  SiUnit u6(SiBaseUnit::METER, 1);
  EXPECT_NE(u5, u6);
  EXPECT_NE(u6, u5);
  SiUnit u7(SiBaseUnit::METER, 2);
  EXPECT_NE(u6, u7);
  EXPECT_NE(u7, u6);
}

TEST(SiUnitTest, Arithmetic) {
  SiUnit unit = SiUnit::Kilogram() * SiUnit::Meter().Power(-2);
  EXPECT_THAT(unit.units(), UnorderedElementsAre(Pair(SiBaseUnit::KILOGRAM, 1),
                                                 Pair(SiBaseUnit::METER, -2)));
  SiUnit kg = SiUnit::Kilogram();
  SiUnit ikg = kg.Invert();
  SiUnit ikg2 = kg.Power(-1);
  EXPECT_NE(kg, ikg);
  EXPECT_NE(kg, ikg2);
  EXPECT_NE(ikg, kg);
  EXPECT_NE(ikg2, kg);
  EXPECT_EQ(ikg, ikg2);
  EXPECT_EQ(ikg2, ikg);
  SiUnit m = SiUnit::Meter();
  SiUnit m2 = m * m;
  EXPECT_THAT(m2.units(), ElementsAre(Pair(SiBaseUnit::METER, 2)));
  EXPECT_EQ(m2 / m, m);
  EXPECT_EQ(m2 / m2, SiUnit::Unitless());
  EXPECT_EQ(unit.Invert(), unit.Power(-1));
}

}  // namespace
}  // namespace cycling
