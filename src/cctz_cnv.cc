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

#include "src/cctz.h"

#include <cstdlib>

#include "src/cctz_impl.h"

namespace cctz {

TimeZone UTCTimeZone() {
  TimeZone tz;
  LoadTimeZone("UTC", &tz);
  return tz;
}

TimeZone LocalTimeZone() {
  const char* zone = std::getenv("TZ");
  if (zone != nullptr) {
    if (*zone == ':') ++zone;
  } else {
    zone = "localtime";
  }
  TimeZone tz;
  if (!LoadTimeZone(zone, &tz)) {
    LoadTimeZone("UTC", &tz);
  }
  return tz;
}

bool LoadTimeZone(const std::string& name, TimeZone* tz) {
  return TimeZone::Impl::LoadTimeZone(name, tz);
}

Breakdown BreakTime(const time_point<seconds64>& tp, const TimeZone& tz) {
  return TimeZone::Impl::get(tz).BreakTime(tp);
}

time_point<seconds64> MakeTime(int64_t year, int mon, int day,
                               int hour, int min, int sec,
                               const TimeZone& tz) {
  return MakeTimeInfo(year, mon, day, hour, min, sec, tz).pre;
}

TimeInfo MakeTimeInfo(int64_t year, int mon, int day,
                      int hour, int min, int sec,
                      const TimeZone& tz) {
  return TimeZone::Impl::get(tz).MakeTimeInfo(year, mon, day, hour, min, sec);
}

}  // namespace cctz
