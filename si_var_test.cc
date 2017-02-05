#include "si_var.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace cycling {
namespace {

TEST(SiVarTest, Ctor) {
  SiVar var;
  SiVar var2(SiUnit(SiBaseUnit::METER, 2), 1);
  SiVar var3(var2);
  var = var2;
  EXPECT_EQ(var, var2);
  EXPECT_EQ(var, var3);
}

TEST(SiVarTest, OpPlusAndMinus) {
  SiVar var(SiUnit::Meter(), 1);
  EXPECT_EQ(var + var, SiVar(SiUnit::Meter(), 1) * 2);
  SiVar var2(SiUnit::Kilogram() / SiUnit::Meter().Power(2), 1);
  EXPECT_EQ((var2 + var2).coef(), 2);
  EXPECT_EQ((var2 + var2).unit(), var2.unit());
  var2 += var2;
  EXPECT_EQ(var2.coef(), 2);
  EXPECT_EQ((var2 - var2).coef(), 0);
  EXPECT_EQ((var2 - var2).unit(), var2.unit());
}

TEST(SiVarTest, OpTimesAndDiv) {
  SiVar var = SiVar(SiUnit::Meter() / SiUnit::Second().Power(2), 10);
  EXPECT_EQ((var * 10).coef(), var.coef() * 10);
  EXPECT_EQ((var * 10).unit(), var.unit());
  EXPECT_EQ((var / 5).coef(), var.coef() / 5);
  EXPECT_EQ((var / 5).unit(), var.unit());
  SiVar var2 = SiVar(SiUnit::Second(), 2);
  EXPECT_EQ((var * var2).coef(), var.coef() * var2.coef());
  EXPECT_EQ((var * var2).unit(), var.unit() * var2.unit());
  EXPECT_EQ((var / var2).coef(), var.coef() / var2.coef());
  EXPECT_EQ((var / var2).unit(), var.unit() / var2.unit());
  var /= var2;
  EXPECT_EQ(var, SiVar(SiUnit::Meter() / SiUnit::Second(), 5));
  SiVar var1 = var;
  var1 *= -1;
  EXPECT_EQ(var1.coef(), -var.coef());
  EXPECT_EQ(var1.unit(), var.unit());
}

TEST(SiVarTest, NamedOps) {
  SiVar var0 = SiVar(SiUnit::Meter() / SiUnit::Second().Power(2), 10);
  EXPECT_EQ(var0.Invert(), SiVar(var0.unit().Invert(), 1 / 10.0));
  EXPECT_EQ(var0.Power(3), SiVar(var0.unit().Power(3), 1000));
  SiVar var1 = var0;
  var1 *= -1;
  EXPECT_EQ(var1.coef(), -var0.coef());
  EXPECT_EQ(var1.unit(), var0.unit());
  EXPECT_EQ(var1.Abs().coef(), var0.coef());
  EXPECT_EQ(var1.Abs().unit(), var0.unit());
}

}  // namespace
}  // namespace cycling
