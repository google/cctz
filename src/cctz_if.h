// Copyright 2015 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//     Unless required by applicable law or agreed to in writing, software
//     distributed under the License is distributed on an "AS IS" BASIS,
//     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
//     implied.
//     See the License for the specific language governing permissions and
//     limitations under the License.

#ifndef CCTZ_IF_H_
#define CCTZ_IF_H_

#include <memory>
#include <string>

#include "src/cctz.h"

namespace cctz {

// A simple interface used to hide time-zone complexities from TimeZone::Impl.
// Subclasses implement the functions for civil-time conversions in the zone.
class TimeZoneIf {
 public:
  // A factory function for TimeZoneIf implementations.
  static std::unique_ptr<TimeZoneIf> Load(const std::string& name);

  virtual ~TimeZoneIf() {}

  virtual Breakdown BreakTime(const time_point<seconds64>& tp) const = 0;
  virtual TimeInfo MakeTimeInfo(int64_t year, int mon, int day,
                                int hour, int min, int sec) const = 0;

 protected:
  TimeZoneIf() {}
};

// Converts tp to a count of seconds since the Unix epoch.
inline int64_t ToUnixSeconds(const time_point<seconds64>& tp) {
  return (tp - std::chrono::time_point_cast<seconds64>(
                   std::chrono::system_clock::from_time_t(0)))
      .count();
}

// Converts a count of seconds since the Unix epoch to a time_point<seconds64>.
inline time_point<seconds64> FromUnixSeconds(int64_t t) {
  return std::chrono::time_point_cast<seconds64>(
             std::chrono::system_clock::from_time_t(0)) +
         seconds64(t);
}

}  // namespace cctz

#endif  // CCTZ_IF_H_
