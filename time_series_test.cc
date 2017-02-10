#include "time_series.h"

#include <chrono>
#include <functional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "measurement.h"

namespace cycling {
namespace {

using TimePoint = TimeSeries::TimePoint;

TimePoint Now() { return std::chrono::system_clock::now(); }
Measurement Hr(const int bpm) {
  return Measurement(Measurement::HEART_RATE, bpm);
}
Measurement Power(const int watts) {
  return Measurement(Measurement::POWER, watts);
}
Measurement Gear(const int gear) {
  return Measurement(Measurement::GEAR, gear);
}
Measurement Cals(const int cals) {
  return Measurement(Measurement::TOTAL_JOULES, cals * 4.184);
}
Measurement Dist(const double km) {
  return Measurement(Measurement::TOTAL_DISTANCE, km * 1000.0);
}
Measurement Cadence(const int rpm) {
  return Measurement(Measurement::CADENCE, rpm);
}

class TimeSeriesTest : public ::testing::Test {
 public:
  TimeSeriesTest() : now_(Now()), second_(std::chrono::seconds(1)) {
    time_samples_[0] = TimeSample(now_)
                           .Add(Hr(120))
                           .Add(Power(200))
                           .Add(Gear(13))
                           .Add(Cadence(90))
                           .Add(Dist(10.0));

    time_samples_[1] = TimeSample(now_ + second_)
                           .Add(Hr(121))
                           .Add(Power(205))
                           .Add(Gear(13))
                           .Add(Cadence(90))
                           .Add(Dist(10.01));

    time_samples_[2] = TimeSample(now_ + second_ * 2)
                           .Add(Hr(121))
                           .Add(Cals(500))
                           .Add(Gear(13))
                           .Add(Cadence(90))
                           .Add(Dist(10.02));

    time_samples_[3] = TimeSample(now_ + second_ * 3)
                           .Add(Hr(121))
                           .Add(Power(205))
                           .Add(Gear(13))
                           .Add(Cadence(90))
                           .Add(Dist(10.02));

    time_samples_[4] = TimeSample(now_ + second_ * 4)
                           .Add(Hr(121))
                           .Add(Power(205))
                           .Add(Gear(13))
                           .Add(Cadence(90))
                           .Add(Dist(10.02));

    time_samples_[5] = TimeSample(now_ + second_ * 5)
                           .Add(Hr(121))
                           .Add(Power(205))
                           .Add(Gear(13))
                           .Add(Cadence(90))
                           .Add(Dist(10.02));
    for (const auto& sample : time_samples_) {
      time_series_.Add(sample);
    }
  }
  ~TimeSeriesTest() override = default;

 protected:
  const TimePoint now_;
  const std::chrono::seconds second_;
  TimeSeries time_series_;
  TimeSample time_samples_[6];
  const int kSize = sizeof(time_samples_) / sizeof(time_samples_[0]);
};

TEST_F(TimeSeriesTest, Times) {
  EXPECT_EQ(time_series_.BeginTime(), now_);
  EXPECT_EQ(time_series_.EndTime(), now_ + second_ * 5);
}

TEST_F(TimeSeriesTest, Visit) {
  int index, sample_index;
  std::vector<double> expected;
  auto visitor = [&index, this](const TimeSample& sample) {
    ASSERT_GE(index, 0);
    ASSERT_LT(index, kSize);
    EXPECT_EQ(sample, time_samples_[index]);
    ++index;
  };
  auto visitor2 = [&index, &sample_index, &expected, this](
      const TimePoint& time, const double coef) {
    ASSERT_GE(index, 0);
    ASSERT_LT(index, kSize);
    ASSERT_GE(sample_index, 0);
    ASSERT_LT(sample_index, expected.size()) << coef;
    EXPECT_EQ(coef, expected[sample_index]) << sample_index;
    ++sample_index;
    ++index;
  };

  index = 0;
  time_series_.Visit(now_, now_ + second_ * kSize, visitor);
  EXPECT_EQ(index, kSize);

  sample_index = 0;
  index = 0;
  expected = {120, 121, 121, 121, 121, 121};
  time_series_.Visit(now_, now_ + second_ * kSize, Measurement::HEART_RATE,
                     visitor2);
  EXPECT_EQ(sample_index, expected.size());

  index = 0;
  time_series_.Visit(now_ + second_ * index, now_ + second_ * 4, visitor);
  EXPECT_EQ(index, 5);

  index = 2;
  time_series_.Visit(now_ + second_ * index, now_ + second_ * kSize, visitor);
  EXPECT_EQ(index, kSize);

  index = 2;
  time_series_.Visit(now_ + second_ * 2, now_ + second_ * 4, visitor);
  EXPECT_EQ(index, 5);

  sample_index = 0;
  index = 2;
  expected = {205, 205, 205};
  time_series_.Visit(now_ + second_ * 2, now_ + second_ * kSize,
                     Measurement::POWER, visitor2);
  EXPECT_EQ(sample_index, expected.size());

  sample_index = 0;
  index = 0;
  expected = {500 * 4.184};
  time_series_.Visit(now_, now_ + second_ * kSize, Measurement::TOTAL_JOULES,
                     visitor2);
  EXPECT_EQ(sample_index, expected.size());

  index = 0;
  time_series_.Visit(now_ - second_ * kSize, now_ + second_ * kSize * 2,
                     visitor);
  EXPECT_EQ(index, kSize);
}

}  // namespace
}  // namespace cycling
