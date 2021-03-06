#include "grapher.h"

#include <cmath>

#include <chrono>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "time_series.h"

namespace cycling {
namespace {

using Duration = std::chrono::system_clock::duration;
using Time = std::chrono::system_clock::time_point;

const double kMinHr = 39;
const double kMaxHr = 185;
const double kSamplePeriod = 100;
const Duration kWindow = std::chrono::seconds(30);
const Duration kIncrement = std::chrono::seconds(1);
const Duration kLookBehind = std::chrono::seconds(3);
const int kNumFrames = 30;
const int kNumSamples = 1200;

TEST(GrapherTest, Plot) {
  TimeSeries time_series;
  const Time start = std::chrono::system_clock::now();

  for (int i = 0; i < kNumSamples; ++i) {
    const double hr =
        kMinHr + (kMaxHr - kMinHr) * std::sin(M_PI * (i / kSamplePeriod) * 2);
    time_series.Add(TimeSample(start + std::chrono::seconds(i),
                               Measurement(Measurement::HEART_RATE, hr)));
  }

  for (int i = 0; i < kNumSamples; ++i) {
    for (int frame = 0; frame < kNumFrames; ++frame) {
      Grapher::Graph graph = Grapher::Plot(
          time_series, start + std::chrono::seconds(i),
          kWindow, kIncrement, kLookBehind,
          Measurement::HEART_RATE,
          1.0, frame / static_cast<double>(kNumFrames));
      EXPECT_GE(graph.labels.size(), 1);
      EXPECT_FALSE(graph.points.empty());
    }
  }
}

}  // namespace
}  // namespace cycling

