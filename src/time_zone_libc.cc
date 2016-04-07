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

#include "time_zone_libc.h"

#include <chrono>
#include <cstdint>
#include <ctime>

// Define OFFSET(tm) and ABBR(tm) for your platform to return the UTC
// offset and zone abbreviation after a call to localtime_r().
#if defined(linux)
# if defined(__USE_BSD)
#  define OFFSET(tm) ((tm).tm_gmtoff)
#  define ABBR(tm)   ((tm).tm_zone)
# else
#  define OFFSET(tm) ((tm).__tm_gmtoff)
#  define ABBR(tm)   ((tm).__tm_zone)
# endif
#elif defined(__APPLE__)
# define OFFSET(tm) ((tm).tm_gmtoff)
# define ABBR(tm)   ((tm).tm_zone)
#elif defined(__sun)
# define OFFSET(tm) ((tm).tm_isdst > 0 ? altzone : timezone)
# define ABBR(tm)   (tzname[(tm).tm_isdst > 0])
#elif defined(_WIN32) || defined(_WIN64)
# define OFFSET(tm) (_timezone + ((tm).tm_isdst > 0 ? 60 * 60 : 0))
# define ABBR(tm)   (_tzname[(tm).tm_isdst > 0])
#else
# define OFFSET(tm) (timezone + ((tm).tm_isdst > 0 ? 60 * 60 : 0))
# define ABBR(tm)   (tzname[(tm).tm_isdst > 0])
#endif

namespace cctz {

TimeZoneLibC::TimeZoneLibC(const std::string& name) {
  local_ = (name == "localtime");
  if (!local_) {
    // TODO: Support "UTC-05:00", for example.
    offset_ = 0;
    abbr_ = "UTC";
  }
}

Breakdown TimeZoneLibC::BreakTime(const time_point<sys_seconds>& tp) const {
  Breakdown bd;
  std::time_t t = ToUnixSeconds(tp);
  std::tm tm;
  if (local_) {
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    bd.offset = OFFSET(tm);
    bd.abbr = ABBR(tm);
  } else {
#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    bd.offset = offset_;
    bd.abbr = abbr_;
  }
  bd.year = tm.tm_year + 1900;
  bd.month = tm.tm_mon + 1;
  bd.day = tm.tm_mday;
  bd.hour = tm.tm_hour;
  bd.minute = tm.tm_min;
  bd.second = tm.tm_sec;
  bd.weekday = (tm.tm_wday ? tm.tm_wday : 7);
  bd.yearday = tm.tm_yday + 1;
  bd.is_dst = tm.tm_isdst > 0;
  return bd;
}

namespace {

// Normalize *val so that 0 <= *val < base, returning any carry.
int NormalizeField(int base, int* val, bool* normalized) {
  int carry = *val / base;
  *val %= base;
  if (*val < 0) {
    carry -= 1;
    *val += base;
  }
  if (carry != 0) *normalized = true;
  return carry;
}

bool IsLeap(int64_t year) {
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

// The month lengths in non-leap and leap years respectively.
const int kDaysPerMonth[2][1+12] = {
  {-1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {-1, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

// The number of days in non-leap and leap years respectively.
const int kDaysPerYear[2] = {365, 366};

// Map a (normalized) Y/M/D to the number of days before/after 1970-01-01.
// See http://howardhinnant.github.io/date_algorithms.html#days_from_civil.
std::time_t DayOrdinal(int64_t year, int month, int day) {
  year -= (month <= 2 ? 1 : 0);
  const std::time_t era = (year >= 0 ? year : year - 399) / 400;
  const int yoe = static_cast<int>(year - era * 400);
  const int doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;  // shift epoch to 1970-01-01
}

}  // namespace

TimeInfo TimeZoneLibC::MakeTimeInfo(int64_t year, int mon, int day,
                                    int hour, int min, int sec) const {
  bool normalized = false;
  std::time_t t;
  if (local_) {
    // Does not handle SKIPPED/AMBIGUOUS or huge years.
    std::tm tm;
    tm.tm_year = static_cast<int>(year - 1900);
    tm.tm_mon = mon - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_isdst = -1;
    t = std::mktime(&tm);
    if (tm.tm_year != year - 1900 || tm.tm_mon != mon - 1 ||
        tm.tm_mday != day || tm.tm_hour != hour ||
        tm.tm_min != min || tm.tm_sec != sec) {
      normalized = true;
    }
  } else {
    min += NormalizeField(60, &sec, &normalized);
    hour += NormalizeField(60, &min, &normalized);
    day += NormalizeField(24, &hour, &normalized);
    mon -= 1;  // months are one-based
    year += NormalizeField(12, &mon, &normalized);
    mon += 1;  // restore [1:12]
    year += (mon > 2 ? 1 : 0);
    int year_len = kDaysPerYear[IsLeap(year)];
    while (day > year_len) {
      day -= year_len;
      year += 1;
      year_len = kDaysPerYear[IsLeap(year)];
    }
    while (day <= 0) {
      year -= 1;
      day += kDaysPerYear[IsLeap(year)];
    }
    year -= (mon > 2 ? 1 : 0);
    bool leap_year = IsLeap(year);
    while (day > kDaysPerMonth[leap_year][mon]) {
      day -= kDaysPerMonth[leap_year][mon];
      if (++mon > 12) {
        mon = 1;
        year += 1;
        leap_year = IsLeap(year);
      }
    }
    t = ((((DayOrdinal(year, mon, day) * 24) + hour) * 60) + min) * 60 + sec;
  }
  TimeInfo ti;
  ti.kind = time_zone::civil_lookup::UNIQUE;
  ti.pre = ti.trans = ti.post = FromUnixSeconds(t);
  ti.normalized = normalized;
  return ti;
}

}  // namespace cctz
