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

#ifndef CCTZ_H_
#define CCTZ_H_

#include <cassert>
#include <chrono>
#include <limits>

#include "civil_time.h"
#include "time_zone.h"

namespace cctz {
inline namespace deprecated_v1_api {
// DEPRECATED: Will be deleted on 2016-10-01.
//
// This header defines the CCTZ v1 API, as it was when originally announced in
// September 2015. The v2 API is declared in two headers: include/civil_time.h
// and include/time_zone.h. For help migrating to the v2 API, see
// https://github.com/google/cctz/wiki/Migrating-from-V1-to-V2-interface
#if CCTZ_ACK_V1_DEPRECATION != 1
#pragma message                                                              \
    "This CCTZ v1 API is depreated and will be deleted 2016-10-01. See "     \
    "https://github.com/google/cctz/wiki/Migrating-from-V1-to-V2-interface " \
    "for details about migrating to the v2 API. "                            \
    "To disable this warning, define CCTZ_ACK_V1_DEPRECATION=1"
#endif

using TimeZone = ::cctz::time_zone;
inline TimeZone UTCTimeZone() { return utc_time_zone(); }
inline TimeZone LocalTimeZone() { return local_time_zone(); }
inline bool LoadTimeZone(const std::string& s, TimeZone* tz) {
  return load_time_zone(s, tz);
}

struct Breakdown {
  int64_t year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int weekday;
  int yearday;
  int offset;
  bool is_dst;
  std::string abbr;
};
template <typename D>
inline Breakdown BreakTime(const time_point<D>& tp, const TimeZone& tz) {
  // Assert that the time point does not require a civil year beyond
  // the limits of a signed 32-bit value, as dictated by the v2 API.
  // -67768100567884800 == -2147483648-01-02 00:00:00 +00:00
  //  67767976233446399 ==  2147483647-12-30 23:59:59 +00:00
  assert(std::chrono::time_point_cast<sys_seconds>(tp) >=
         std::chrono::time_point_cast<sys_seconds>(
             std::chrono::system_clock::from_time_t(0)) +
             sys_seconds(-67768100567884800));
  assert(std::chrono::time_point_cast<sys_seconds>(tp) <=
         std::chrono::time_point_cast<sys_seconds>(
             std::chrono::system_clock::from_time_t(0)) +
             sys_seconds(67767976233446399));

  const auto al = tz.lookup(tp);
  const auto cs = al.cs;
  const auto yd = get_yearday(civil_day(cs));
  int wd{};
  switch (get_weekday(civil_day(cs))) {
    case weekday::monday:
      wd = 1;
      break;
    case weekday::tuesday:
      wd = 2;
      break;
    case weekday::wednesday:
      wd = 3;
      break;
    case weekday::thursday:
      wd = 4;
      break;
    case weekday::friday:
      wd = 5;
      break;
    case weekday::saturday:
      wd = 6;
      break;
    case weekday::sunday:
      wd = 7;
      break;
  }
  return {cs.year(), cs.month(), cs.day(),  cs.hour(), cs.minute(), cs.second(),
          wd,        yd,         al.offset, al.is_dst, al.abbr};
}

using seconds64 = std::chrono::duration<int64_t, std::chrono::seconds::period>;
inline time_point<seconds64> MakeTime(int64_t year, int mon, int day, int hour,
                                      int min, int sec, const TimeZone& tz) {
  assert(year < std::numeric_limits<int>::max());
  return tz.lookup(civil_second(year, mon, day, hour, min, sec)).pre;
}

struct TimeInfo {
  enum class Kind {
    UNIQUE,
    SKIPPED,
    REPEATED,
  } kind;
  time_point<seconds64> pre;
  time_point<seconds64> trans;
  time_point<seconds64> post;
  bool normalized;
};
inline TimeInfo MakeTimeInfo(int64_t y, int m, int d, int hh, int mm, int ss,
                             const TimeZone& tz) {
  assert(y < std::numeric_limits<decltype(civil_second().year())>::max());
  const civil_second cs(y, m, d, hh, mm, ss);
  const bool norm = cs.year() != y || cs.month() != m || cs.day() != d ||
                    cs.hour() != hh || cs.minute() != mm || cs.second() != ss;
  const auto cl = tz.lookup(cs);
  TimeInfo::Kind kind;
  switch (cl.kind) {
    case time_zone::civil_lookup::civil_kind::UNIQUE:
      kind = TimeInfo::Kind::UNIQUE;
      break;
    case time_zone::civil_lookup::civil_kind::SKIPPED:
      kind = TimeInfo::Kind::SKIPPED;
      break;
    case time_zone::civil_lookup::civil_kind::REPEATED:
      kind = TimeInfo::Kind::REPEATED;
      break;
  }
  return {kind, cl.pre, cl.trans, cl.post, norm};
}

template <typename D>
inline std::string Format(const std::string& fmt, const time_point<D>& tp,
                          const TimeZone& tz) {
  return format(fmt, tp, tz);
}

template <typename D>
inline bool Parse(const std::string& fmt, const std::string& input,
                  const TimeZone& tz, time_point<D>* tpp) {
  return parse(fmt, input, tz, tpp);
}

}  // namespace deprecated_v1_api
}  // namespace cctz

#endif  // CCTZ_H_
