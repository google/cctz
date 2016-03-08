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

// A library for computing with civil times (Y-M-D h:m:s) in a
// time-zone-independent manner. These classes may help with rounding,
// iterating, and arithmetic, while avoiding complications like DST.

#ifndef CCTZ_CIVIL_TIME_H_
#define CCTZ_CIVIL_TIME_H_

#include <limits>

namespace cctz {

namespace detail {
#include "civil_time_detail.h"
}  // namespace detail

// The types civil_year, civil_month, civil_day, civil_hour, civil_minute,
// and civil_second each implement the concept of a "civil time" that
// is aligned to the boundary of the unit indicated by the type's name.
// A "civil time" is a time-zone-independent representation of the six
// fields---year, month, day, hour, minute, and second---that follow the
// rules of the proleptic Gregorian calendar with exactly 24-hour days,
// 60-minute hours, and 60-second minutes.
//
// Each of these six types implement the same API, so learning to use any
// one of them will translate to all of the others. Some of the following
// examples will use civil_day and civil_month, but any other civil-time
// types may be substituted.
//
// CONSTRUCTION:
//
// All civil-time types allow convenient construction from various sets
// of arguments. Unspecified fields default to their minimum value.
// Specified fields that are out of range are first normalized (e.g.,
// Oct 32 is normalized to Nov 1). Specified fields that are smaller
// than the indicated alignment unit are set to their minimum value
// (i.e., 1 for months and days; 0 for hours, minutes, and seconds).
//
//   civil_day a(2015, 6, 28);  // Construct from Y-M-D
//   civil_day b(2015, 6, 28, 9, 9, 9);  // H:M:S floored to 00:00:00
//   civil_day c(2015);   // Defaults to Jan 1
//   civil_month m(a);    // Floors the given day to a month boundary
//
// VALUE SEMANTICS:
//
// All civil-time types are small, value types that should be copied,
// assigned to, and passed to functions by value.
//
//   void F(civil_day day);  // Accepts by value, not const reference
//   civil_day a(2015, 6, 28);
//   civil_day b;
//   b = a;                  // Copy
//   F(b);                   // Passed by value
//
// PROPERTIES:
//
// All civil-time types have accessors for all six of the civil-time
// fields: year, month, day, hour, minute, and second.
//
//   civil_day a(2015, 6, 28);
//   assert(a.year() == 2015);
//   assert(a.month() == 6);
//   assert(a.day() == 28);
//   assert(a.hour() == 0);
//   assert(a.minute() == 0);
//   assert(a.second() == 0);
//
// ARITHMETIC:
//
// All civil-time types allow natural arithmetic expressions that respect
// the type's indicated alignment. For example, adding 1 to a civil_month
// adds one month, and adding 1 to a civil_day adds one day.
//
//   civil_day a(2015, 6, 28);
//   ++a;                  // a = 2015-06-29  (--a is also supported)
//   a++;                  // a = 2015-06-30  (a-- is also supported)
//   civil_day b = a + 1;  // b = 2015-07-01  (a - 1 is also supported)
//   civil_day c = 1 + b;  // c = 2015-07-02
//   int n = c - a;        // n = 2 (days)
//
// COMPARISON:
//
// All civil-time types may be compared with each other, regardless of
// the type's alignment. Comparison is equivalent to comparing all six
// civil-time fields.
//
//   // Iterates all the days of June.
//   // (Compares a civil_day with a civil_month)
//   for (civil_day day(2015, 6, 1); day < civil_month(2015, 7); ++day) {
//     // ...
//   }
//
using civil_year = detail::civil_year;
using civil_month = detail::civil_month;
using civil_day = detail::civil_day;
using civil_hour = detail::civil_hour;
using civil_minute = detail::civil_minute;
using civil_second = detail::civil_second;

// An enum class with members monday, tuesday, wednesday, thursday,
// friday, saturday, and sunday.
using detail::weekday;

// Returns the weekday for the given civil_day.
//
//   civil_day a(2015, 8, 13);
//   weekday wd = get_weekday(a);  // wd == weekday::thursday
//
using detail::get_weekday;

// Returns the civil_day that strictly follows or precedes the argument,
// and that falls on the given weekday.
//
// For example, given:
//
//     August 2015
// Su Mo Tu We Th Fr Sa
//                    1
//  2  3  4  5  6  7  8
//  9 10 11 12 13 14 15
// 16 17 18 19 20 21 22
// 23 24 25 26 27 28 29
// 30 31
//
//   civil_day a(2015, 8, 13);  // get_weekday(a) == weekday::thursday
//   civil_day b = next_weekday(a, weekday::thursday);  // b = 2015-08-20
//   civil_day c = prev_weekday(a, weekday::thursday);  // c = 2015-08-06
//
//   civil_day d = ...
//   // Gets the following Thursday if d is not already Thursday
//   civil_day thurs1 = PrevWeekday(d, weekday::thursday) + 7;
//   // Gets the previous Thursday if d is not already Thursday
//   civil_day thurs2 = NextWeekday(d, weekday::thursday) - 7;
//
using detail::next_weekday;
using detail::prev_weekday;

// Returns the day-of-year for the given civil_day.
//
//   civil_day a(2015, 1, 1);
//   int yd_jan_1 = get_yearday(a);   // yd_jan_1 = 1
//   civil_day b(2015, 12, 31);
//   int yd_dec_31 = get_yearday(b);  // yd_dec_31 = 365
//
using detail::get_yearday;

}  // namespace cctz

#endif  // CCTZ_CIVIL_TIME_H_
