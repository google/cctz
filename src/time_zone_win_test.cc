// Copyright 2025 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   https://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include <chrono>
#include <string>

#include "cctz/civil_time.h"
#include "gtest/gtest.h"
#include "time_zone_if.h"
#include "time_zone_win.h"

namespace cctz {
namespace {

time_point<seconds> UtcToTp(const civil_second& cs) {
  return std::chrono::time_point_cast<seconds>(
             std::chrono::system_clock::from_time_t(0)) +
         seconds(cs - civil_second(1970, 1, 1, 0, 0, 0));
}

civil_second cs(year_t y, int m, int d, int hh) {
  return civil_second(y, m, d, hh);
}

struct CivilTransitionData {
  std::time_t unix_seconds;
  civil_second from;
  civil_second to;
  CivilTransitionData() : unix_seconds(0), from(), to() {}
  CivilTransitionData(std::time_t unix_seconds_, civil_second from_,
                      civil_second to_)
      : unix_seconds(unix_seconds_), from(from_), to(to_) {}
};

void ExpectNextTransitions(const TimeZoneIf* tz, year_t start_year,
                           const std::vector<CivilTransitionData>& data) {
  auto tp = UtcToTp(civil_second(start_year, 1, 1, 0, 0, 0));
  size_t i = 0;
  while (true) {
    time_zone::civil_transition trans;
    if (!tz->NextTransition(tp, &trans)) {
      break;
    }
    if (i >= data.size()) {
      break;
    }
    const auto& expected = data[i];
    EXPECT_EQ(expected.from, trans.from);
    EXPECT_EQ(expected.to, trans.to);
    time_zone::civil_lookup to_cl = tz->MakeTime(trans.to);
    tp = to_cl.trans;
    EXPECT_EQ(FromUnixSeconds(expected.unix_seconds), tp);
    ++i;
  }
}

void ExpectNoTransitionAfter(const TimeZoneIf* tz,
                             std::int_fast64_t unix_seconds) {
  time_zone::civil_transition trans;
  EXPECT_FALSE(tz->NextTransition(FromUnixSeconds(unix_seconds), &trans));
}

void ExpectLocalTime(const TimeZoneIf* tz, civil_second utc,
                     civil_second expected_local, bool expected_dst,
                     const std::string& expected_abbr) {
  auto al = tz->BreakTime(UtcToTp(utc));
  EXPECT_EQ(al.cs, expected_local);
  EXPECT_EQ(al.offset, expected_local - utc);
  EXPECT_EQ(al.is_dst, expected_dst);
  EXPECT_EQ(al.abbr, expected_abbr);
}

TEST(TimeZoneWin, NoOffset) {
  const WinTimeZoneRegistryInfo info = {
      {
          {0, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
      },
      0};
  auto tzif = cctz::MakeTimeZoneFromWinRegistry(info);
  ExpectLocalTime(tzif.get(), cs(2025, 8, 1, 0), cs(2025, 8, 1, 0), false,
                  "GMT");
}

TEST(TimeZoneWin, QuarterHourOffset) {
  const WinTimeZoneRegistryInfo info = {
      {
          {15, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
      },
      0};
  auto tzif = cctz::MakeTimeZoneFromWinRegistry(info);
  ExpectLocalTime(tzif.get(), civil_second(2025, 8, 1, 0),
                  civil_second(2025, 7, 31, 23, 45), false, "GMT-0015");
}

TEST(TimeZoneWin, FixedOffset) {
  // America/Phoenix
  const WinTimeZoneRegistryInfo info = {
      {
          {420, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
      },
      0};
  auto tzif = cctz::MakeTimeZoneFromWinRegistry(info);
  ExpectNoTransitionAfter(tzif.get(), 0);
  ExpectLocalTime(tzif.get(), cs(2025, 8, 1, 0), cs(2025, 7, 31, 17), false,
                  "GMT-07");
}

TEST(TimeZoneWin, YearDependentDst) {
  // America/Los_Angeles
  const WinTimeZoneRegistryInfo info = {
      {
          // 2006
          {480, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 4, 0, 1, 2, 0, 0, 0}},
          {480, 0, -60, {0, 11, 0, 1, 2, 0, 0, 0}, {0, 3, 0, 2, 2, 0, 0, 0}},
          // TZI
          {480, 0, -60, {0, 11, 0, 1, 2, 0, 0, 0}, {0, 3, 0, 2, 2, 0, 0, 0}},
      },
      2006};
  auto tzif = cctz::MakeTimeZoneFromWinRegistry(info);
  ExpectLocalTime(tzif.get(), cs(2005, 3, 15, 0), cs(2005, 3, 14, 16), false,
                  "GMT-08");
  ExpectLocalTime(tzif.get(), cs(2006, 3, 15, 0), cs(2006, 3, 14, 16), false,
                  "GMT-08");
  ExpectLocalTime(tzif.get(), cs(2007, 3, 15, 0), cs(2007, 3, 14, 17), true,
                  "GMT-07");
}

TEST(TimeZoneWin, NonDSTtoDSTtoNonDST) {
  // Asia/Ulaanbaatar
  const std::vector<Win32TimeZoneRegistryEntry> entries = {
      // 2014
      {-480, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
      {-480, 0, -60, {0, 9, 5, 5, 23, 59, 59, 999}, {0, 3, 6, 5, 2, 0, 0, 0}},
      {-480, 0, -60, {0, 9, 5, 4, 23, 59, 59, 999}, {0, 3, 6, 5, 2, 0, 0, 0}},
      {-480, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
      // TZI
      {-480, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
  };
  const WinTimeZoneRegistryInfo info = {entries, 2014};
  const std::vector<CivilTransitionData> next_transitions = {
      {1427479200, cs(2015, 3, 28, 2), cs(2015, 3, 28, 3)},
      {1443193200, cs(2015, 9, 26, 0), cs(2015, 9, 25, 23)},
      {1458928800, cs(2016, 3, 26, 2), cs(2016, 3, 26, 3)},
      {1474642800, cs(2016, 9, 24, 0), cs(2016, 9, 23, 23)},
  };
  ExpectNextTransitions(tzif.get(), 2010, next_transitions);
  ExpectNoTransitionAfter(tzif.get(), 1474642800);
}

TEST(TimeZoneWin, DiscontinuousYearBoundary) {
  // Europe/Volgograd
  // https://techcommunity.microsoft.com/blog/dstblog/2020-time-zone-updates-for-volgograd-russia-now-available/2234995
  const WinTimeZoneRegistryInfo info = {
      {
          // 2010
          {-180, 0, -60, {0, 10, 0, 5, 3, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0}},
          {-180, 0, -60, {0, 1, 6, 1, 0, 0, 0, 0}, {0, 3, 0, 5, 2, 0, 0, 0}},
          {-240, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
          {-240, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
          {-180, 0, -60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 1, 3, 1, 0, 0, 0, 0}},
          // 2015
          {-180, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
          {-180, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
          {-180, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
          {-240, 0, 60, {0, 10, 0, 5, 2, 0, 0, 0}, {0, 1, 1, 1, 0, 0, 0, 0}},
          {-240, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
          // 2020
          {-240, 0, -60, {0, 12, 0, 5, 2, 0, 0, 0}, {0, 1, 3, 1, 0, 0, 0, 0}},
          {-180, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
          // TZI
          {-180, 0, -60, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}},
      },
      2010};

  auto tzif = cctz::MakeTimeZoneFromWinRegistry(info);

  // https://github.com/dotnet/runtime/issues/118915
  const std::vector<CivilTransitionData> expected = {
      {1269730800, cs(2010, 3, 28, 2), cs(2010, 3, 28, 3)},
      {1288479600, cs(2010, 10, 31, 3), cs(2010, 10, 31, 2)},
      {1301180400, cs(2011, 3, 27, 2), cs(2011, 3, 27, 3)},
      {1414274400, cs(2014, 10, 26, 2), cs(2014, 10, 26, 1)},
      {1540681200, cs(2018, 10, 28, 2), cs(2018, 10, 28, 3)},
      {1577822400, cs(2020, 1, 1, 0), cs(2020, 1, 1, 1)},  // nonexistent
      {1609016400, cs(2020, 12, 27, 2), cs(2020, 12, 27, 1)},
      {1609444800, cs(2021, 1, 1, 0), cs(2020, 12, 31, 23)},  // nonexistent
  };
  ExpectNextTransitions(tzif.get(), 2010, expected);
  ExpectNoTransitionAfter(tzif.get(), 1609444800);
}

}  // namespace
}  // namespace cctz
