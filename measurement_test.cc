#include "measurement.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "si_unit.h"

using TimePoint = cycling::TimeSample::TimePoint;

namespace cycling {
namespace {

TEST(MeasurementTest, Test) {
  Measurement m0(Measurement::NO_TYPE, 1);
  Measurement m1(Measurement::NO_TYPE, 2);
  Measurement m2(Measurement::HEART_RATE, 150);
  Measurement m3(Measurement::HRV, 8.2);
  Measurement m4(Measurement::POWER, 250);
  Measurement m5(Measurement::POWER, SiVar(SiUnit::Watt(), 250));

  EXPECT_EQ(m0, m0);
  EXPECT_EQ(m1, m1);
  EXPECT_EQ(m2, m2);
  EXPECT_EQ(m3, m3);

  EXPECT_LT(m0, m1);
  EXPECT_GT(m1, m0);
  EXPECT_LE(m0, m0);
  EXPECT_GE(m2, m2);

  EXPECT_NE(m0, m3);
  EXPECT_NE(m1, m2);

  EXPECT_EQ(m4, m5);

  EXPECT_EQ(m4.value(), m5.value());

  EXPECT_DEATH(Measurement(Measurement::TOTAL_JOULES, SiVar(SiUnit::Unitless(), 10));
}

}  // namespace
}  // namespace cycling
