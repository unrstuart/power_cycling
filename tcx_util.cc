#include "tcx_util.h"

#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "measurement.h"
#include "si_base_unit.h"
#include "si_unit.h"
#include "si_var.h"
#include "status.h"
#include "str_util.h"
#include "time_sample.h"
#include "time_series.h"
#include "xml_util.h"

namespace cycling {

namespace {

using SampleHandler = std::function<Status(const XmlNode*, TimeSample*)>;
using SeriesHandler = std::function<Status(const XmlNode*, TimeSeries*)>;

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

enum TcxEntity {
  TCX_UNKNOWN,
  TCX_TEXT,
  ACTIVITIES,
  ACTIVITY,
  ALTITUDE_METERS,
  AUTHOR,
  AVERAGE_HEART_RATE_BPM,
  AVG_BIKE_CADENCE,
  AVG_RUN_CADENCE,
  AVG_SPEED,
  AVG_WATTS,
  BUILD,
  BUILD_MAJOR,
  BUILD_MINOR,
  CADENCE,
  CALORIES,
  CREATOR,
  DISTANCE_METERS,
  EXTENSIONS,
  HEART_RATE_BPM,
  ID,
  INTENSITY,
  LX,
  LANG_ID,
  LAP,
  LATITUDE_DEGREES,
  LONGITUDE_DEGREES,
  MAX_BIKE_CADENCE,
  MAX_RUN_CADENCE,
  MAX_WATTS,
  MAXIMUM_HEART_RATE_BPM,
  MAXIMUM_SPEED,
  NAME,
  PART_NUMBER,
  POSITION,
  PRODUCT_ID,
  RUN_CADENCE,
  SPEED,
  STEPS,
  TPX,
  TIME,
  TOTAL_TIME_SECONDS,
  TRACK,
  TRACKPOINT,
  TRAINING_CENTER_DATABASE,
  TRIGGER_METHOD,
  UNIT_ID,
  VALUE,
  VERSION,
  VERSION_MAJOR,
  VERSION_MINOR,
  WATTS,
};

const std::initializer_list<std::pair<std::string, TcxEntity>> kEntities = {
    {"(unknown)", TCX_UNKNOWN},
    {"(text)", TCX_TEXT},
    {"activities", ACTIVITIES},
    {"activity", ACTIVITY},
    {"altitudemeters", ALTITUDE_METERS},
    {"author", AUTHOR},
    {"averageheartratebpm", AVERAGE_HEART_RATE_BPM},
    {"avgbikecadence", AVG_RUN_CADENCE},
    {"avgruncadence", AVG_RUN_CADENCE},
    {"avgspeed", AVG_SPEED},
    {"avgwatts", AVG_WATTS},
    {"build", BUILD},
    {"buildmajor", BUILD_MAJOR},
    {"buildminor", BUILD_MINOR},
    {"cadence", CADENCE},
    {"calories", CALORIES},
    {"creator", CREATOR},
    {"distancemeters", DISTANCE_METERS},
    {"extensions", EXTENSIONS},
    {"heartratebpm", HEART_RATE_BPM},
    {"id", ID},
    {"intensity", INTENSITY},
    {"lx", LX},
    {"langid", LANG_ID},
    {"lap", LAP},
    {"latitudedegrees", LATITUDE_DEGREES},
    {"longitudedegrees", LONGITUDE_DEGREES},
    {"maxbikecadence", MAX_BIKE_CADENCE},
    {"maxruncadence", MAX_RUN_CADENCE},
    {"maxwatts", MAX_WATTS},
    {"maximumheartratebpm", MAXIMUM_HEART_RATE_BPM},
    {"maximumspeed", MAXIMUM_SPEED},
    {"name", NAME},
    {"partnumber", PART_NUMBER},
    {"position", POSITION},
    {"productid", PRODUCT_ID},
    {"runcadence", RUN_CADENCE},
    {"speed", SPEED},
    {"steps", STEPS},
    {"tpx", TPX},
    {"time", TIME},
    {"totaltimeseconds", TOTAL_TIME_SECONDS},
    {"track", TRACK},
    {"trackpoint", TRACKPOINT},
    {"trainingcenterdatabase", TRAINING_CENTER_DATABASE},
    {"triggermethod", TRIGGER_METHOD},
    {"unitid", UNIT_ID},
    {"value", VALUE},
    {"version", VERSION},
    {"versionmajor", VERSION_MAJOR},
    {"versionminor", VERSION_MINOR},
    {"watts", WATTS},
};

TcxEntity GetTcxEntity(const XmlNode* node) {
  static std::map<std::string, TcxEntity> entities;
  if (entities.empty()) {
    for (const auto& p : kEntities) entities[p.first] = p.second;
  }
  if (node == nullptr) return TCX_UNKNOWN;
  if (node->type == XmlNode::FREE_TEXT) return TCX_TEXT;
  auto it = entities.find(ToLowercase(node->name));
  if (it == entities.end()) return TCX_UNKNOWN;
  return it->second;
}

std::string GetTcxEntityName(const TcxEntity entity) {
  static const std::vector<std::pair<std::string, TcxEntity>> entities(
      kEntities);
  if (entity < 0 || entity >= entities.size()) return "(unknown)";
  return entities[entity].first;
}

Status EntityMatches(const XmlNode* node, const XmlNode* parent,
                     const TcxEntity entity) {
  if (node == nullptr) {
    return Status::FailureStatus(
        StrCat("Expected node to be of type ", GetTcxEntityName(entity),
               " but it was null. Parent=",
               parent == nullptr ? std::string("nullptr") : parent->name, "."));
  }
  const TcxEntity ent = GetTcxEntity(node);
  if (ent == entity) return Status::OkStatus();
  return Status::FailureStatus(StrCat("Expected node to be ",
                                      GetTcxEntityName(entity), " but it was ",
                                      node->name, "."));
}

void PrintPathToRoot(const XmlNode* node) {
  if (node == nullptr) return;
  std::cerr << "name: " << node->name << std::endl;
  PrintPathToRoot(node->parent);
}

Status ChildCountEquals(const XmlNode* node, const int num_children) {
  if (node == nullptr) {
    return Status::FailureStatus(
        "Expected node to have children, but node is null.");
  }
  if (node->children.size() != num_children) {
    PrintPathToRoot(node);
    return Status::FailureStatus(StrCat("Expected ", node->name, " to have ",
                                        num_children, " children, but it has ",
                                        node->children.size(), "."));
  }
  return Status::OkStatus();
}

Status ChildCountGreaterThan(const XmlNode* node, const int num_children) {
  if (node == nullptr) {
    return Status::FailureStatus(
        "Expected node to have children, but node is null.");
  }
  if (node->children.size() <= num_children) {
    return Status::FailureStatus(
        StrCat("Expected ", node->name, " to have more than ", num_children,
               " children, but it has ", node->children.size(), "."));
  }
  return Status::OkStatus();
}

// Returns OK iff node contains one child, and it's a text node. If so, also
// changes *contained_text to point to the text.
Status ContainsOneTextChild(const XmlNode* node,
                            const std::string** contained_text) {
  if (node->children.size() != 1) {
    return Status::FailureStatus(
        StrCat(node->name, " should have one child (text) but it has ",
               node->children.size()));
  }
  const XmlNode* kid = node->children[0].get();
  RETURN_IF_ERROR(EntityMatches(kid, node, TCX_TEXT));
  RETURN_IF_ERROR(ChildCountEquals(kid, 0));
  *contained_text = &kid->text;
  return Status::OkStatus();
}

Status ParseActivities(const XmlNode* node, TimeSeries* series);
Status ParseActivity(const XmlNode* node, TimeSeries* series);
Status ParseAltitudeMeters(const XmlNode* node, TimeSample* sample);
Status ParseAuthor(const XmlNode* node, TimeSeries* series);
Status ParseAverageHeartRateBpm(const XmlNode* node, TimeSeries* series);
Status ParseAvgRunCadence(const XmlNode* node, TimeSeries* series);
Status ParseAvgSpeed(const XmlNode* node, TimeSeries* series);
Status ParseAvgWatts(const XmlNode* node, TimeSeries* series);
Status ParseBuild(const XmlNode* node, TimeSeries* series);
Status ParseBuildMajor(const XmlNode* node, TimeSeries* series);
Status ParseBuildMinor(const XmlNode* node, TimeSeries* series);
Status ParseCadence(const XmlNode* node, TimeSeries* series);
Status ParseCadenceSample(const XmlNode* node, TimeSample* sample);
Status ParseCalories(const XmlNode* node, TimeSeries* series);
Status ParseCreator(const XmlNode* node, TimeSeries* series);
Status ParseDistanceMeters(const XmlNode* node, TimeSeries* series);
Status ParseDistanceMetersSample(const XmlNode* node, TimeSample* sample);
Status ParseExtensions(const XmlNode* node, TimeSeries* series);
Status ParseExtensionsSample(const XmlNode* node, TimeSample* sample);
Status ParseHeartRateBpm(const XmlNode* node, TimeSample* sample);
Status ParseId(const XmlNode* node, TimeSeries* series);
Status ParseIntensity(const XmlNode* node, TimeSeries* series);
Status ParseLx(const XmlNode* node, TimeSeries* series);
Status ParseLangID(const XmlNode* node, TimeSeries* series);
Status ParseLap(const XmlNode* node, TimeSeries* series);
Status ParseLatitudeDegrees(const XmlNode* node, TimeSample* sample);
Status ParseLongitudeDegrees(const XmlNode* node, TimeSample* sample);
Status ParseMaxRunCadence(const XmlNode* node, TimeSeries* series);
Status ParseMaximumHeartRateBpm(const XmlNode* node, TimeSeries* series);
Status ParseMaximumSpeed(const XmlNode* node, TimeSeries* series);
Status ParseMaxWatts(const XmlNode* node, TimeSeries* series);
Status ParseName(const XmlNode* node, TimeSeries* series);
Status ParsePartNumber(const XmlNode* node, TimeSeries* series);
Status ParsePosition(const XmlNode* node, TimeSample* sample);
Status ParseProductID(const XmlNode* node, TimeSeries* series);
Status ParseRunCadence(const XmlNode* node, TimeSample* sample);
Status ParseSpeed(const XmlNode* node, TimeSample* sample);
Status ParseSteps(const XmlNode* node, TimeSeries* series);
Status ParseTpx(const XmlNode* node, TimeSample* sample);
Status ParseTime(const XmlNode* node, TimeSample* sample);
Status ParseTotalTimeSeconds(const XmlNode* node, TimeSeries* series);
Status ParseTrack(const XmlNode* node, TimeSeries* series);
Status ParseTrackpoint(const XmlNode* node, TimeSample* sample);
Status ParseTrainingCenterDatabase(const XmlNode* node, TimeSeries* series);
Status ParseTriggerMethod(const XmlNode* node, TimeSeries* series);
Status ParseUnitId(const XmlNode* node, TimeSeries* series);
Status ParseValue(const XmlNode* node, TimeSeries* series);
Status ParseVersion(const XmlNode* node, TimeSeries* series);
Status ParseVersionMajor(const XmlNode* node, TimeSeries* series);
Status ParseVersionMinor(const XmlNode* node, TimeSeries* series);
Status ParseWatts(const XmlNode* node, TimeSample* sample);

Status ParseTrainingCenterDatabase(const XmlNode* node, TimeSeries* series) {
  RETURN_IF_ERROR(ChildCountGreaterThan(node, 0));
  const XmlNode* activities = nullptr;
  for (const auto& kid : node->children) {
    if (EntityMatches(kid.get(), node, ACTIVITIES).ok()) {
      activities = kid.get();
      break;
    }
  }
  if (activities == nullptr) {
    return Status::FailureStatus(
        StrCat("Could not find an activities node under ", node->name, "."));
  }
  RETURN_IF_ERROR(EntityMatches(activities, node, ACTIVITIES));
  RETURN_IF_ERROR(ParseActivities(activities, series));
  return Status::OkStatus();
}

Status ParseActivities(const XmlNode* node, TimeSeries* series) {
  RETURN_IF_ERROR(ChildCountEquals(node, 1));
  const XmlNode* activity = node->children[0].get();
  RETURN_IF_ERROR(EntityMatches(activity, node, ACTIVITY));
  RETURN_IF_ERROR(ParseActivity(activity, series));
  return Status::OkStatus();
}

Status ParseActivity(const XmlNode* node, TimeSeries* series) {
  // The activity should have an ID and one or more laps.
  RETURN_IF_ERROR(ChildCountGreaterThan(node, 1));
  const XmlNode* kid = node->children[0].get();
  RETURN_IF_ERROR(EntityMatches(kid, node, ID));
  RETURN_IF_ERROR(ParseId(kid, series));
  for (auto it = ++node->children.begin(); it != node->children.end(); ++it) {
    if (EntityMatches(it->get(), node, LAP).ok()) {
      RETURN_IF_ERROR(ParseLap(it->get(), series));
    } else if (EntityMatches(it->get(), node, CREATOR).ok()) {
      RETURN_IF_ERROR(ParseCreator(node, series));
      return Status::OkStatus();
    } else {
      return Status::FailureStatus(
          StrCat("Expected either LAP or CREATOR as a child of ", node->name,
                 " but got ", GetTcxEntityName(GetTcxEntity(it->get()))));
    }
  }
  return Status::OkStatus();
}

// Prints the text in the single child node of node. Returns false and prints
// nothing if the node does not contain exactly one child (or that child is not
// text).
Status ParseAndPrintTextParent(const std::string& prefix, const XmlNode* node,
                               TimeSeries* series) {
  const std::string* text;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &text));
  std::cout << prefix << ": '" << *text << "'.\n";
  return Status::OkStatus();
}

Status ParseId(const XmlNode* node, TimeSeries* series) {
  RETURN_IF_ERROR(ParseAndPrintTextParent("TCX Activity ID", node, series));
  return Status::OkStatus();
}

std::string ConvertToString(const XmlNode* node, const int indent) {
  if (node == nullptr) return "";
  std::string ret;
  for (int i = 0; i < indent; ++i) ret += "  ";
  if (node->type == XmlNode::FREE_TEXT) return ret + node->text + "\n";
  if (node->type == XmlNode::ROOT) return ret + "?xml\n";
  ret += node->name;
  for (const auto& attr : node->attrs) {
    ret += StrCat(" ", attr.first, "=", attr.second);
  }
  ret += "\n";
  for (const auto& kid : node->children) {
    ret += ConvertToString(kid.get(), indent + 1);
  }
  return ret;
}

template <typename HandlerMap, typename OutputData>
Status ParseWithHandlers(const XmlNode* node, const HandlerMap& handlers,
                         OutputData* data) {
  for (const auto& kid : node->children) {
    const TcxEntity entity = GetTcxEntity(kid.get());
    auto it = handlers.find(entity);
    if (it == handlers.end()) {
      return Status::FailureStatus(StrCat("Unexpected child of ", node->name,
                                          ": ", GetTcxEntityName(entity), "\n",
                                          ConvertToString(kid.get(), 0)));
    }
    RETURN_IF_ERROR(it->second(kid.get(), data));
  }
  return Status::OkStatus();
}

Status ParseLap(const XmlNode* node, TimeSeries* series) {
  RETURN_IF_ERROR(ChildCountGreaterThan(node, 0));
  const std::map<TcxEntity, SeriesHandler> kHandlers = {
      {TOTAL_TIME_SECONDS, ParseTotalTimeSeconds},
      {DISTANCE_METERS, ParseDistanceMeters},
      {MAXIMUM_SPEED, ParseMaximumSpeed},
      {CADENCE, ParseCadence},
      {CALORIES, ParseCalories},
      {EXTENSIONS, ParseExtensions},
      {AVERAGE_HEART_RATE_BPM, ParseAverageHeartRateBpm},
      {MAXIMUM_HEART_RATE_BPM, ParseMaximumHeartRateBpm},
      {INTENSITY, ParseIntensity},
      {TRIGGER_METHOD, ParseTriggerMethod},
      {TRACK, ParseTrack},
  };
  std::cout << "parsing new lap:\n";
  RETURN_IF_ERROR(ParseWithHandlers(node, kHandlers, series));
  return Status::OkStatus();
}

Status ExtractDouble(const std::string& str, double* d) {
  int n;
  if (sscanf(str.c_str(), "%lf%n", d, &n) != 1 || n != str.size()) {
    return Status::FailureStatus(
        StrCat("Expected string to contain one float, got ", str, " instead."));
  }
  return Status::OkStatus();
}

Status ExtractInt(const std::string& str, int* i) {
  int n;
  if (sscanf(str.c_str(), "%d%n", i, &n) != 1 || n != str.size()) {
    return Status::FailureStatus(
        StrCat("Expected string to contain one int, got ", str, " instead."));
  }
  return Status::OkStatus();
}

Status ExtractTime(const std::string& str, TimeSample::TimePoint* time) {
  int y, m, d, h, min, s, ms, n;
  if (sscanf(str.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d.%dZ%n", &y, &m, &d, &h, &min,
             &s, &ms, &n) != 7 ||
      n < str.size()) {
    if (sscanf(str.c_str(), "%4d-%2d-%2dT%2d:%2d:%2dZ%n", &y, &m, &d, &h, &min,
               &s, &n) != 6 ||
        n < str.size()) {
      return Status::FailureStatus(
          StrCat("Expected a Time value, got '", str, "' instead."));
    }
  }
  struct tm c_time;
  c_time.tm_sec = s;
  c_time.tm_min = min;
  c_time.tm_hour = h;
  c_time.tm_mday = d;
  c_time.tm_mon = m - 1;
  c_time.tm_year = y - 1900;
  *time = std::chrono::system_clock::from_time_t(mktime(&c_time));
  return Status::OkStatus();
}

Status ParseTotalTimeSeconds(const XmlNode* node, TimeSeries* series) {
  const std::string* time;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &time));
  double d;
  RETURN_IF_ERROR(ExtractDouble(*time, &d));
  SiVar var(SiUnit::Second(), d);
  std::cout << "  Total Lap Time: " << var << "\n";
  return Status::OkStatus();
}

Status ParseDistanceMeters(const XmlNode* node, TimeSeries* series) {
  const std::string* dist;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &dist));
  double d;
  RETURN_IF_ERROR(ExtractDouble(*dist, &d));
  SiVar var(SiUnit::Meter(), d);
  std::cout << "  Total Lap Distance: " << var << "\n";
  return Status::OkStatus();
}

Status ParseMaximumSpeed(const XmlNode* node, TimeSeries* series) {
  const std::string* speed;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &speed));
  double d;
  RETURN_IF_ERROR(ExtractDouble(*speed, &d));
  SiVar var(SiUnit::MetersPerSecond(), d);
  std::cout << "  Maximum Lap Speed: " << var << "\n";
  return Status::OkStatus();
}

Status ParseCadence(const XmlNode* node, TimeSeries* series) {
  const std::string* steps;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &steps));
  int spm;
  RETURN_IF_ERROR(ExtractInt(*steps, &spm));
  std::cout << "  Average Cadence: " << spm << "\n";
  return Status::OkStatus();
}

Status ParseCadenceSample(const XmlNode* node, TimeSample* sample) {
  const std::string* rpm_text;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &rpm_text));
  int rpm;
  RETURN_IF_ERROR(ExtractInt(*rpm_text, &rpm));
  sample->Add(Measurement(Measurement::CADENCE, rpm));
  return Status::OkStatus();
}

Status ParseCalories(const XmlNode* node, TimeSeries* series) {
  const std::string* cals;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &cals));
  double d;
  RETURN_IF_ERROR(ExtractDouble(*cals, &d));
  SiVar var = 4.184 * d * SiVar::Joule();
  std::cout << "  Total Lap Energy: " << var << "\n";
  return Status::OkStatus();
}

Status ParseHeartRate(const XmlNode* node, int* bpm) {
  RETURN_IF_ERROR(ChildCountEquals(node, 1));
  RETURN_IF_ERROR(ChildCountEquals(node->children[0].get(), 1));
  const XmlNode* kid = node->children[0].get();
  RETURN_IF_ERROR(EntityMatches(kid, node, VALUE));
  const std::string* bpm_text;
  RETURN_IF_ERROR(ContainsOneTextChild(kid, &bpm_text));
  RETURN_IF_ERROR(ExtractInt(*bpm_text, bpm));
  return Status::OkStatus();
}

Status ParseAverageHeartRateBpm(const XmlNode* node, TimeSeries* series) {
  int bpm;
  RETURN_IF_ERROR(ParseHeartRate(node, &bpm));
  std::cout << "  Average Lap HR: " << bpm << " (bpm)\n";
  return Status::OkStatus();
}

Status ParseMaximumHeartRateBpm(const XmlNode* node, TimeSeries* series) {
  int bpm;
  RETURN_IF_ERROR(ParseHeartRate(node, &bpm));
  std::cout << "  Maximum Lap HR: " << bpm << " (bpm)\n";
  return Status::OkStatus();
}

Status ParseIntensity(const XmlNode* node, TimeSeries* series) {
  RETURN_IF_ERROR(ParseAndPrintTextParent("  Lap Intensity", node, series));
  return Status::OkStatus();
}

Status ParseTriggerMethod(const XmlNode* node, TimeSeries* series) {
  RETURN_IF_ERROR(ParseAndPrintTextParent("  Trigger Method", node, series));
  return Status::OkStatus();
}

Status ParseTrack(const XmlNode* node, TimeSeries* series) {
  RETURN_IF_ERROR(ChildCountGreaterThan(node, 0));
  for (const auto& kid : node->children) {
    RETURN_IF_ERROR(EntityMatches(kid.get(), node, TRACKPOINT));
    TimeSample sample;
    RETURN_IF_ERROR(ParseTrackpoint(kid.get(), &sample));
  }
  return Status::OkStatus();
}

Status ParseTrackpoint(const XmlNode* node, TimeSample* sample) {
  RETURN_IF_ERROR(ChildCountGreaterThan(node, 0));
  const std::map<TcxEntity, SampleHandler> kHandlers = {
      {TIME, ParseTime},
      {POSITION, ParsePosition},
      {CADENCE, ParseCadenceSample},
      {ALTITUDE_METERS, ParseAltitudeMeters},
      {DISTANCE_METERS, ParseDistanceMetersSample},
      {HEART_RATE_BPM, ParseHeartRateBpm},
      {EXTENSIONS, ParseExtensionsSample},
  };
  RETURN_IF_ERROR(ParseWithHandlers(node, kHandlers, sample));
  return Status::OkStatus();
}

Status ParseTime(const XmlNode* node, TimeSample* sample) {
  RETURN_IF_ERROR(ChildCountEquals(node, 1));
  const std::string* time;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &time));
  TimeSample::TimePoint time_point;
  RETURN_IF_ERROR(ExtractTime(*time, &time_point));
  sample->set_time(time_point);
  return Status::OkStatus();
}

Status ParsePosition(const XmlNode* node, TimeSample* sample) {
  RETURN_IF_ERROR(ChildCountEquals(node, 2));
  const std::map<TcxEntity, SampleHandler> kHandlers = {
      {LATITUDE_DEGREES, ParseLatitudeDegrees},
      {LONGITUDE_DEGREES, ParseLongitudeDegrees},
  };
  RETURN_IF_ERROR(ParseWithHandlers(node, kHandlers, sample));
  return Status::OkStatus();
}

Status ParseLatitudeDegrees(const XmlNode* node, TimeSample* sample) {
  const std::string* gps;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &gps));
  double d;
  RETURN_IF_ERROR(ExtractDouble(*gps, &d));
  sample->Add(Measurement(Measurement::DEGREES_LATITUDE, d));
  return Status::OkStatus();
}

Status ParseLongitudeDegrees(const XmlNode* node, TimeSample* sample) {
  const std::string* gps;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &gps));
  double d;
  RETURN_IF_ERROR(ExtractDouble(*gps, &d));
  sample->Add(Measurement(Measurement::DEGREES_LONGITUDE, d));
  return Status::OkStatus();
}

Status ParseAltitudeMeters(const XmlNode* node, TimeSample* sample) {
  const std::string* elev;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &elev));
  double d;
  RETURN_IF_ERROR(ExtractDouble(*elev, &d));
  sample->Add(Measurement(Measurement::ALTITUDE, d));
  return Status::OkStatus();
}

Status ParseDistanceMetersSample(const XmlNode* node, TimeSample* sample) {
  const std::string* elev;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &elev));
  double d;
  RETURN_IF_ERROR(ExtractDouble(*elev, &d));
  sample->Add(Measurement(Measurement::INCREMENTAL_DISTANCE, d));
  return Status::OkStatus();
}

Status ParseHeartRateBpm(const XmlNode* node, TimeSample* sample) {
  int bpm;
  RETURN_IF_ERROR(ParseHeartRate(node, &bpm));
  sample->Add(Measurement(Measurement::HEART_RATE, bpm));
  return Status::OkStatus();
}

Status ParseExtensionsSample(const XmlNode* node, TimeSample* sample) {
  RETURN_IF_ERROR(ChildCountEquals(node, 1));
  const XmlNode* kid = node->children[0].get();
  RETURN_IF_ERROR(EntityMatches(kid, node, TPX));
  RETURN_IF_ERROR(ParseTpx(kid, sample));
  return Status::OkStatus();
}

Status ParseTpx(const XmlNode* node, TimeSample* sample) {
  RETURN_IF_ERROR(ChildCountGreaterThan(node, 0));
  const std::map<TcxEntity, SampleHandler> kHandlers = {
      {SPEED, ParseSpeed},
      {WATTS, ParseWatts},
      {RUN_CADENCE, ParseRunCadence},
  };
  RETURN_IF_ERROR(ParseWithHandlers(node, kHandlers, sample));
  return Status::OkStatus();
}

Status ParseSpeed(const XmlNode* node, TimeSample* sample) {
  const std::string* speed_text;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &speed_text));
  double m_s;
  RETURN_IF_ERROR(ExtractDouble(*speed_text, &m_s));
  sample->Add(Measurement(Measurement::SPEED, m_s));
  return Status::OkStatus();
}

Status ParseWatts(const XmlNode* node, TimeSample* sample) {
  const std::string* watts_text;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &watts_text));
  int watts;
  RETURN_IF_ERROR(ExtractInt(*watts_text, &watts));
  sample->Add(Measurement(Measurement::POWER, watts));
  return Status::OkStatus();
}

Status ParseRunCadence(const XmlNode* node, TimeSample* sample) {
  const std::string* steps;
  RETURN_IF_ERROR(ContainsOneTextChild(node, &steps));
  int spm;
  RETURN_IF_ERROR(ExtractInt(*steps, &spm));
  sample->Add(Measurement(Measurement::CADENCE, spm));
  return Status::OkStatus();
}

Status ParseExtensions(const XmlNode* node, TimeSeries* series) {
  RETURN_IF_ERROR(ChildCountGreaterThan(node, 0));
  for (const auto& kid : node->children) {
    RETURN_IF_ERROR(EntityMatches(kid.get(), node, LX));
    RETURN_IF_ERROR(ParseLx(kid.get(), series));
  }
  return Status::OkStatus();
}

Status ParseLx(const XmlNode* node, TimeSeries* series) {
  RETURN_IF_ERROR(ChildCountGreaterThan(node, 0));
  const std::map<TcxEntity, SeriesHandler> kHandlers = {
      {AVG_RUN_CADENCE, ParseAvgRunCadence},
      {MAX_RUN_CADENCE, ParseMaxRunCadence},
      {AVG_BIKE_CADENCE, ParseAvgRunCadence},
      {MAX_BIKE_CADENCE, ParseMaxRunCadence},
      {AVG_SPEED, ParseAvgSpeed},
      {AVG_WATTS, ParseAvgWatts},
      {MAX_WATTS, ParseMaxWatts},
      {STEPS, ParseSteps},
  };
  RETURN_IF_ERROR(ParseWithHandlers(node, kHandlers, series));
  return Status::OkStatus();
}

Status ParseMaxRunCadence(const XmlNode* node, TimeSeries* series) {
  const std::string* cadence;
  RETURN_IF_ERROR(ChildCountEquals(node, 1));
  RETURN_IF_ERROR(ContainsOneTextChild(node, &cadence));
  int spm;
  RETURN_IF_ERROR(ExtractInt(*cadence, &spm));
  std::cout << "  Maximum Lap Cadence: " << spm << " spm.\n";
  return Status::OkStatus();
}

Status ParseAvgRunCadence(const XmlNode* node, TimeSeries* series) {
  const std::string* cadence;
  RETURN_IF_ERROR(ChildCountEquals(node, 1));
  RETURN_IF_ERROR(ContainsOneTextChild(node, &cadence));
  int spm;
  RETURN_IF_ERROR(ExtractInt(*cadence, &spm));
  std::cout << "  Average Lap Cadence: " << spm << " spm.\n";
  return Status::OkStatus();
}

Status ParseAvgSpeed(const XmlNode* node, TimeSeries* series) {
  const std::string* speed;
  RETURN_IF_ERROR(ChildCountEquals(node, 1));
  RETURN_IF_ERROR(ContainsOneTextChild(node, &speed));
  double m_s;
  RETURN_IF_ERROR(ExtractDouble(*speed, &m_s));
  const SiVar s = m_s * SiVar::MetersPerSecond();
  std::cout << "  Average Lap Speed: " << s << ".\n";
  return Status::OkStatus();
}

Status ParseAvgWatts(const XmlNode* node, TimeSeries* series) {
  const std::string* watts;
  RETURN_IF_ERROR(ChildCountEquals(node, 1));
  RETURN_IF_ERROR(ContainsOneTextChild(node, &watts));
  int power;
  RETURN_IF_ERROR(ExtractInt(*watts, &power));
  const SiVar s = power * SiVar::Watt();
  std::cout << "  Average Lap Watts: " << s << ".\n";
  return Status::OkStatus();
}

Status ParseMaxWatts(const XmlNode* node, TimeSeries* series) {
  const std::string* watts;
  RETURN_IF_ERROR(ChildCountEquals(node, 1));
  RETURN_IF_ERROR(ContainsOneTextChild(node, &watts));
  int power;
  RETURN_IF_ERROR(ExtractInt(*watts, &power));
  const SiVar s = power * SiVar::Watt();
  std::cout << "  Maximum Lap Watts: " << s << ".\n";
  return Status::OkStatus();
}

Status ParseSteps(const XmlNode* node, TimeSeries* series) {
  const std::string* steps;
  RETURN_IF_ERROR(ChildCountEquals(node, 1));
  RETURN_IF_ERROR(ContainsOneTextChild(node, &steps));
  int s;
  RETURN_IF_ERROR(ExtractInt(*steps, &s));
  std::cout << "  Total Lap Steps: " << s << " steps.\n";
  return Status::OkStatus();
}

Status ParseCreator(const XmlNode* node, TimeSeries* series) {
  return Status::OkStatus();
}

Status BeginParse(const XmlNode* node, TimeSeries* series) {
  if (node == nullptr || node->type != XmlNode::ROOT) {
    return Status::FailureStatus(
        StrCat("Expected root to have type XmlNode::ROOT, got ", node->type,
               " instead."));
  }
  node = node->children[0].get();
  // Under the root, we should have a single node.
  if (node == nullptr) {
    return Status::FailureStatus("Root XML node is null.");
  }
  RETURN_IF_ERROR(EntityMatches(node, nullptr, TRAINING_CENTER_DATABASE));
  RETURN_IF_ERROR(ParseTrainingCenterDatabase(node, series));
  return Status::OkStatus();
}

}  // namespace

std::unique_ptr<TimeSeries> ParseTcxFile(const std::string& path) {
  std::unique_ptr<XmlNode> node = xml_util::ParseXmlFile(path);
  if (!node) return nullptr;
  TimeSeries series;
  const Status status = BeginParse(node.get(), &series);
  if (!status.ok()) {
    std::cerr << status << std::endl;
    return nullptr;
  }
  return make_unique<TimeSeries>(std::move(series));
}

}  // namespace cycling
