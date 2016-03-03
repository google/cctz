// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef CCTZ_TIME_ZONE_IF_H_
#define CCTZ_TIME_ZONE_IF_H_

#include <cstdint>
#include <memory>
#include <string>

#include "civil_time.h"
#include "time_zone.h"

namespace cctz {

// The calendar and wall-clock (a.k.a. "civil time") components of a
// time_point in a certain time_zone. A better std::tm.  Note that we
// cannot use time_zone::absolute_lookup because we need a 64-bit year.
struct Breakdown {
  int64_t year;      // year (e.g., 2013)
  int month;         // month of year [1:12]
  int day;           // day of month [1:31]
  int hour;          // hour of day [0:23]
  int minute;        // minute of hour [0:59]
  int second;        // second of minute [0:59]
  int weekday;       // 1==Mon, ..., 7=Sun
  int yearday;       // day of year [1:366]

  // Note: The following fields exist for backward compatibility with older
  // APIs. Accessing these fields directly is a sign of imprudent logic in the
  // calling code. Modern time-related code should only access this data
  // indirectly by way of cctz::format().
  int offset;        // seconds east of UTC
  bool is_dst;       // is offset non-standard?
  std::string abbr;  // time-zone abbreviation (e.g., "PST")
};

// A TimeInfo represents the conversion of year, month, day, hour, minute,
// and second values in a particular time_zone to a time instant.
struct TimeInfo {
  time_zone::civil_lookup::civil_kind kind;
  time_point<sys_seconds> pre;   // Uses the pre-transition offset
  time_point<sys_seconds> trans;
  time_point<sys_seconds> post;  // Uses the post-transition offset
  bool normalized;
};

// A simple interface used to hide time-zone complexities from time_zone::Impl.
// Subclasses implement the functions for civil-time conversions in the zone.
class TimeZoneIf {
 public:
  // A factory function for TimeZoneIf implementations.
  static std::unique_ptr<TimeZoneIf> Load(const std::string& name);

  virtual ~TimeZoneIf() {}

  virtual Breakdown BreakTime(const time_point<sys_seconds>& tp) const = 0;
  virtual TimeInfo MakeTimeInfo(int64_t year, int mon, int day,
                                int hour, int min, int sec) const = 0;

 protected:
  TimeZoneIf() {}
};

// Converts tp to a count of seconds since the Unix epoch.
inline int64_t ToUnixSeconds(const time_point<sys_seconds>& tp) {
  return (tp - std::chrono::time_point_cast<sys_seconds>(
                   std::chrono::system_clock::from_time_t(0)))
      .count();
}

// Converts a count of seconds since the Unix epoch to a
// time_point<sys_seconds>.
inline time_point<sys_seconds> FromUnixSeconds(int64_t t) {
  return std::chrono::time_point_cast<sys_seconds>(
             std::chrono::system_clock::from_time_t(0)) +
         sys_seconds(t);
}

}  // namespace cctz

#endif  // CCTZ_TIME_ZONE_IF_H_
