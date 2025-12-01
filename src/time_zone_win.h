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

#ifndef CCTZ_TIME_ZONE_WIN_H_
#define CCTZ_TIME_ZONE_WIN_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "time_zone_if.h"

namespace cctz {

// A platform-independent redefinition of Windows' SYSTEMTIME structure.
// https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-systemtime
// Intentionally uses uint_fast8_t for several fields with an assumtion that
// data validation is already performed in the data loader layer.
struct WinSystemTime {
  WinSystemTime()
      : year(0),
        month(0),
        day_of_week(0),
        day(0),
        hour(0),
        minute(0),
        second(0),
        milliseconds(0) {}
  WinSystemTime(std::uint_fast16_t year_, std::uint_fast8_t month_,
                std::uint_fast8_t day_of_week_, std::uint_fast8_t day_,
                std::uint_fast8_t hour_, std::uint_fast8_t minute_,
                std::uint_fast8_t second_, std::uint_fast16_t milliseconds_)
      : year(year_),
        month(month_),
        day_of_week(day_of_week_),
        day(day_),
        hour(hour_),
        minute(minute_),
        second(second_),
        milliseconds(milliseconds_) {}

  const std::uint_fast16_t year;
  const std::uint_fast8_t month;
  const std::uint_fast8_t day_of_week;
  const std::uint_fast8_t day;
  const std::uint_fast8_t hour;
  const std::uint_fast8_t minute;
  const std::uint_fast8_t second;
  const std::uint_fast16_t milliseconds;
};

// A platform-independent redefinition of Windows' REG_TZI_FORMAT structure.
// https://learn.microsoft.com/en-us/windows/win32/api/timezoneapi/ns-timezoneapi-time_zone_information
struct WinTimeZoneRegistryEntry {
  WinTimeZoneRegistryEntry()
      : bias(0),
        standard_bias(0),
        daylight_bias(0),
        standard_date(),
        daylight_date() {}
  WinTimeZoneRegistryEntry(std::int_fast32_t bias_,
                           std::int_fast32_t standard_bias_,
                           std::int_fast32_t daylight_bias_,
                           const WinSystemTime& standard_date_,
                           const WinSystemTime& daylight_date_)
      : bias(bias_),
        standard_bias(standard_bias_),
        daylight_bias(daylight_bias_),
        standard_date(standard_date_),
        daylight_date(daylight_date_) {}

  // Base offset in minutes, where UTC == local time + bias.
  const std::int_fast32_t bias;
  // Additional offset in minutes applied to standard time.
  const std::int_fast32_t standard_bias;
  // Additional offset in minutes applied to DST.
  const std::int_fast32_t daylight_bias;
  // Local time (in the previous offset) when the standard time begins.
  const WinSystemTime standard_date;
  // Local time (in the previous offset) when the DST begins.
  const WinSystemTime daylight_date;
};

// A platform-independent data snapshot of Windows Registry Time Zone entries.
struct WinTimeZoneRegistryInfo {
  WinTimeZoneRegistryInfo() : entries(), first_year(0) {}

  WinTimeZoneRegistryInfo(std::vector<WinTimeZoneRegistryEntry> entries_,
                          year_t first_year_)
      : entries(std::move(entries_)), first_year(first_year_) {}

  // This field is also used to indicate whether the object is valid or not.
  //  - Size of 0: Invalid object (e.g. failed to load from the registry).
  //  - Size of 1: No per-year override. `first_year` is ignored.
  //  - Size of N: Per-year override for N years with extrapolations with the
  //               first/last entry.
  std::vector<WinTimeZoneRegistryEntry> entries;
  year_t first_year;
};

// MakeTimeZoneFromWinRegistry does not validate the entries in
// WinTimeZoneRegistryInfo (e.g. invalid date entries).
// In production, LoadWinTimeZoneRegistry() takes care of runtime data
// validations.
std::unique_ptr<TimeZoneIf> MakeTimeZoneFromWinRegistry(
    WinTimeZoneRegistryInfo info);

}  // namespace cctz

#endif  // CCTZ_TIME_ZONE_WIN_H_
