#include "tcx_util.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace cycling {
namespace {

const char k310OutdoorRun[] = "310_outdoor_run.tcx";
const char kFenix3IndoorIntervalsRun[] = "fenix3_indoor_intervals_run.tcx";
const char kFenix3IndoorRide[] = "fenix3_indoor_ride.tcx";
const char kFenix3OutdoorIntervalsRun[] = "fenix3_outdoor_interval_run.tcx";
const char kFenix3OutdoorLongRun[] = "fenix3_outdoor_long_run.tcx";
const char kFenix3OutdoorRide[] = "fenix3_outdoor_ride.tcx";
const char kTrainerroadRide[] = "trainerroad_ride.tcx";

TEST(TcxUtilTest, Parse310TcxRun) {
  EXPECT_NE(ParseTcxFile(k310OutdoorRun).get(), nullptr);
}

TEST(TcxUtilTest, ParseFenix3OutdoorRun) {
  EXPECT_NE(ParseTcxFile(kFenix3OutdoorLongRun).get(), nullptr);
}

TEST(TcxUtilTest, ParseFenix3OutdoorIntervalRun) {
  EXPECT_NE(ParseTcxFile(kFenix3OutdoorIntervalsRun).get(), nullptr);
}

TEST(TcxUtilTest, ParseFenix3IndoorRun) {
  EXPECT_NE(ParseTcxFile(kFenix3IndoorIntervalsRun).get(), nullptr);
}

TEST(TcxUtilTest, ParseFenix3OutdoorRide) {
  EXPECT_NE(ParseTcxFile(kFenix3OutdoorRide).get(), nullptr);
}

TEST(TcxUtilTest, ParseFenix3IndoorRide) {
  EXPECT_NE(ParseTcxFile(kFenix3IndoorRide).get(), nullptr);
}

TEST(TcxUtilTest, ParseTrainerRoadRide) {
  EXPECT_NE(ParseTcxFile(kTrainerroadRide).get(), nullptr);
}

}  // namespace
}  // namespace cycling
