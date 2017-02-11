#include "tcx_util.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace cycling {
namespace {

const char kTestFile[] = "tcx_util_test_data.tcx";

TEST(TcxUtilTest, Parse310TcxRun) {
  ParseTcxFile();
}

TEST(TcxUtilTest, ParseFenix3OutdoorRun) {
  ParseTcxFile();
}

TEST(TcxUtilTest, ParseFenix3OutdoorIntervalRun) {
  ParseTcxFile();
}

TEST(TcxUtilTest, ParseFenix3IndoorRun) {
  ParseTcxFile();
}

TEST(TcxUtilTest, ParseFenix3OutdoorRide) {
  ParseTcxFile();
}

TEST(TcxUtilTest, ParseFenix3IndoorRide) {
  ParseTcxFile();
}

TEST(TcxUtilTest, ParseTrainerRoadRide) {
  ParseTcxFile();
}

}  // namespace
}  // namespace cycling
