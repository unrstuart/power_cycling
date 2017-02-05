#include "time_sample.h"

#include <chrono>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "measurement.h"

using TimePoint = cycling::TimeSample::TimePoint;

namespace cycling {
namespace {

Measurement Bpm180() { return Measurement(Measurement::HEART_RATE, 180); }

Measurement Watts150() {
  return Measurement(Measurement::POWER, SiVar(SiUnit::Watt(), 150));
}

TimePoint Now() { return std::chrono::system_clock::now(); }

TEST(TimeSampleTest, CtorAndEquality) {
  const TimePoint tp = Now();
  const TimePoint tp2 = tp + std::chrono::seconds(10);
  TimeSample s0;
  TimeSample s1(tp);
  TimeSample s2(tp2);
  TimeSample s3(tp, {});
  TimeSample s4(tp2, {});
  TimeSample s5(tp, Bpm180());
  TimeSample s6(tp2, Bpm180());
  TimeSample s7(tp, {Bpm180(), Watts150()});
  TimeSample s8(tp, {Watts150(), Bpm180()});
  TimeSample s9(tp2, {Bpm180(), Watts150()});

  EXPECT_EQ(s0, s0);
  EXPECT_NE(s0, s1);
  EXPECT_NE(s0, s2);
  EXPECT_EQ(s1, s1);
  EXPECT_EQ(s2, s2);
  EXPECT_EQ(s1, s3);
  EXPECT_NE(s1, s2);
  EXPECT_NE(s5, s6);
  EXPECT_EQ(s7, s8);
  EXPECT_NE(s7, s9);
}

TEST(TimeSampleTest, ValueAndAddAndIterator) {
  const TimePoint tp = Now();

  TimeSample s0(tp);

  TimeSample s1 = s0.Add(Bpm180());
  TimeSample s2 = s1.Add(Bpm180());
  TimeSample s3 = s0.Add(Watts150());
  TimeSample s4 = s3.Add(Watts150());
  TimeSample s5 = s2.Add(Watts150());
  TimeSample s6 = s3.Add(Bpm180());

  EXPECT_EQ(s1, s2);
  EXPECT_EQ(s3, s4);
  EXPECT_EQ(s5, s6);

  EXPECT_EQ(s1.value(Measurement::HEART_RATE),
            s2.value(Measurement::HEART_RATE));

  EXPECT_EQ(s1.time(), s2.time());
  EXPECT_EQ(s0.time(), s2.time());

  bool has_power = false, has_hr = false;
  for (const auto& p : s5) {
    if (p.first == Measurement::POWER) has_power = true;
    if (p.first == Measurement::HEART_RATE) has_hr = true;
  }
  EXPECT_TRUE(has_power);
  EXPECT_TRUE(has_hr);
}

}  // namespace
}  // namespace cycling
