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

#include "time_zone_win.h"

#include <algorithm>
#include <chrono>
#include <deque>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "time_zone_fixed.h"
#include "time_zone_if.h"

namespace cctz {
namespace {

struct RawOffsetInfo {
  RawOffsetInfo() : offset_seconds(0), dst(false) {}
  std::int_fast32_t offset_seconds;
  bool dst;
};

// Transitions extracted from WinTimeZoneRegistryEntry (==REG_TZI_FORMAT) for
// the target year. Each WinTimeZoneRegistryEntry can provide up to three
// transitions in a year.
// The most tricky part is that WinTimeZoneRegistryEntry gives us localtime in
// "from" offset whereas corresponding Biases are "to" offset. This means that
// "from" localtime cannot be converted to UTC time without knowing the "from"
// offset.
// See ResolveSystemTime() on how WinTimeZoneRegistryEntry is interpreted.
struct RawTransitionInfo {
  civil_second from_civil_time;
  RawOffsetInfo to;
};

civil_second TpToUtc(const time_point<seconds>& tp) {
  return civil_second(1970, 1, 1, 0, 0, 0) +
         (tp - std::chrono::time_point_cast<seconds>(
                   std::chrono::system_clock::from_time_t(0)))
             .count();
}

time_point<seconds> UtcToTp(const civil_second& cs) {
  return std::chrono::time_point_cast<seconds>(
             std::chrono::system_clock::from_time_t(0)) +
         seconds(cs - civil_second(1970, 1, 1, 0, 0, 0));
}

const char* kCommonAbbrs[] = {
    "GMT-14", "GMT-1330", "GMT-13", "GMT-1230", "GMT-12", "GMT-1130",
    "GMT-11", "GMT-1030", "GMT-10", "GMT-0930", "GMT-09", "GMT-0830",
    "GMT-08", "GMT-0730", "GMT-07", "GMT-0630", "GMT-06", "GMT-0530",
    "GMT-05", "GMT-0430", "GMT-04", "GMT-0330", "GMT-03", "GMT-0230",
    "GMT-02", "GMT-0130", "GMT-01", "GMT+0030", "GMT",    "GMT+0030",
    "GMT+01", "GMT+0130", "GMT+02", "GMT+0230", "GMT+03", "GMT+0330",
    "GMT+04", "GMT+0430", "GMT+05", "GMT+0530", "GMT+06", "GMT+0630",
    "GMT+07", "GMT+0730", "GMT+08", "GMT+0830", "GMT+09", "GMT+0930",
    "GMT+10", "GMT+1030", "GMT+11", "GMT+1130", "GMT+12", "GMT+1230",
    "GMT+13", "GMT+1330", "GMT+14",
};

const char* GetCommonAbbreviation(std::int_fast32_t offset_seconds) {
  if (offset_seconds % 1800 == 0) {
    const std::int_fast32_t halfhour_offset = offset_seconds / 1800;
    if (-28 <= halfhour_offset && halfhour_offset <= 28) {
      return kCommonAbbrs[halfhour_offset + 28];
    }
  }
  return nullptr;
}

class AbbreviationMap {
 public:
  AbbreviationMap() = default;
  AbbreviationMap(std::vector<std::int_fast32_t> index_key,
                  std::vector<std::string> index_value)
      : index_key_(std::move(index_key)),
        index_value_(std::move(index_value)) {}

  const char* Get(std::int_fast32_t offset_seconds) const {
    const char* common_abbr = GetCommonAbbreviation(offset_seconds);
    if (common_abbr != nullptr) {
      return common_abbr;
    }
    for (size_t i = 0; i < index_key_.size(); ++i) {
      if (index_key_[i] == offset_seconds) {
        // The returned pointer remains to be valid as long as we do not modify
        // index_value_.
        return index_value_[i].c_str();
      }
    }
    return "";
  }

 private:
  const std::vector<std::int_fast32_t> index_key_;
  const std::vector<std::string> index_value_;
};

class AbbreviationMapBuilder {
 public:
  AbbreviationMapBuilder() = default;

  void Add(const WinTimeZoneRegistryEntry& info) {
    AddInternal(-60 * info.bias);
    if (info.standard_bias != 0) {
      AddInternal(-60 * (info.bias + info.standard_bias));
    }
    if (info.daylight_bias != 0) {
      AddInternal(-60 * (info.bias + info.daylight_bias));
    }
  }

  AbbreviationMap Build() {
    extra_offsets_.shrink_to_fit();
    std::vector<std::int_fast32_t> result;
    extra_offsets_.swap(result);

    std::vector<std::string> abbrs;
    abbrs.reserve(result.size());
    for (const std::int_fast32_t offset : result) {
      const char* common_abbr = GetCommonAbbreviation(offset);
      if (common_abbr == nullptr) {
        abbrs.push_back("GMT" + cctz::FixedOffsetToAbbr(cctz::seconds(offset)));
      }
    }
    return AbbreviationMap(std::move(result), std::move(abbrs));
  }

 private:
  void AddInternal(std::int_fast32_t offset_seconds) {
    if (GetCommonAbbreviation(offset_seconds) != nullptr) {
      return;  // Already exists as a common abbreviation.
    }
    for (size_t i = 0; i < extra_offsets_.size(); ++i) {
      if (extra_offsets_[i] == offset_seconds) {
        return;  // Already exists.
      }
    }
    extra_offsets_.push_back(offset_seconds);
  }

  std::vector<std::int_fast32_t> extra_offsets_;
};

struct LocalTimeInfo {
  LocalTimeInfo() : offset_seconds(0), is_dst(false) {}
  civil_second civil_time;
  std::int_fast32_t offset_seconds;
  bool is_dst;
};

struct TimeOffsetInfo {
  TimeOffsetInfo() : kind(time_zone::civil_lookup::UNIQUE) {}

  LocalTimeInfo from;
  LocalTimeInfo to;
  time_point<seconds> tp;
  time_zone::civil_lookup::civil_kind kind;

  const civil_second& earlier_cs() const {
    // Equivalent to std::min(from.civil_time, to.civil_time)
    return kind == time_zone::civil_lookup::REPEATED ? to.civil_time
                                                     : from.civil_time;
  }
  const civil_second& later_cs() const {
    // Equivalent to std::max(from.civil_time, to.civil_time)
    return kind == time_zone::civil_lookup::REPEATED ? from.civil_time
                                                     : to.civil_time;
  }
};

const cctz::weekday kWeekdays[] = {
    cctz::weekday::sunday,    cctz::weekday::monday,   cctz::weekday::tuesday,
    cctz::weekday::wednesday, cctz::weekday::thursday, cctz::weekday::friday,
    cctz::weekday::saturday};

class TimeZoneRegistry {
 public:
  static TimeZoneRegistry Load(WinTimeZoneRegistryInfo info) {
    AbbreviationMapBuilder abbr_map_builder;
    for (const auto& info : info.entries) {
      abbr_map_builder.Add(info);
    }
    return TimeZoneRegistry(std::move(info), abbr_map_builder.Build());
  }

  const year_t FirstYear() const {
    if (timezone_info_.entries.size() < 2) {
      return 0;
    }
    return timezone_info_.first_year;
  }
  const year_t LastYear() const {
    if (timezone_info_.entries.size() < 2) {
      return 0;
    }
    // The last entry is the fixed (or the latest) one.
    return timezone_info_.first_year +
           static_cast<std::uint_fast32_t>(timezone_info_.entries.size() - 2);
  }
  const bool IsAvailable() const { return !timezone_info_.entries.empty(); }
  const bool IsYearDependent() const {
    return timezone_info_.entries.size() >= 2;
  }
  const bool IsFixed() const {
    return timezone_info_.entries.size() == 1
               ? IsFixedTimeZone(timezone_info_.entries.back())
               : false;
  }

  const bool StartsWithFixed() const {
    return timezone_info_.entries.empty()
               ? false
               : IsFixedTimeZone(timezone_info_.entries.front());
  }
  const bool EndsWithFixed() const {
    return timezone_info_.entries.empty()
               ? false
               : IsFixedTimeZone(timezone_info_.entries.back());
  }

  const char* GetAbbreviation(std::int_fast32_t offset_seconds) const {
    return abbr_map_.Get(offset_seconds);
  }

  std::int_fast32_t GetFixedOffset() const {
    if (timezone_info_.entries.empty()) {
      return 0;
    }
    const auto& base = timezone_info_.entries.back();
    return -60 * base.bias;
  }

  std::deque<TimeOffsetInfo> GetOffsetInfo(year_t year_start,
                                           year_t year_end) const {
    if (!IsAvailable() || year_start > year_end) {
      return {};
    }
    std::deque<TimeOffsetInfo> result;
    RawOffsetInfo last_base_info;
    for (year_t year = year_start - 1; year <= year_end; ++year) {
      const auto first_year = timezone_info_.first_year;
      const auto& entries = timezone_info_.entries;
      const size_t index =
          year <= first_year
              ? 0
              : std::min<size_t>(year - first_year, entries.size() - 1);
      const auto transitions = ParseTimeZoneInfo(entries[index], year);
      if (year == year_start - 1) {
        last_base_info = transitions.back().to;
        continue;
      }
      for (const auto& transition : transitions) {
        TimeOffsetInfo info;
        info.from.civil_time = transition.from_civil_time;
        info.from.offset_seconds = last_base_info.offset_seconds;
        info.from.is_dst = last_base_info.dst;
        info.tp =
            UtcToTp(transition.from_civil_time - last_base_info.offset_seconds);
        info.to.offset_seconds = transition.to.offset_seconds;
        info.to.is_dst = transition.to.dst;
        const std::int_fast32_t offset_diff =
            transition.to.offset_seconds - last_base_info.offset_seconds;
        info.to.civil_time = info.from.civil_time + offset_diff;
        if (offset_diff > 0) {
          info.kind = time_zone::civil_lookup::SKIPPED;
        } else if (offset_diff == 0) {
          if (!result.empty()) {
            const auto& last_info = result.back();
            if (last_info.to.offset_seconds == info.from.offset_seconds &&
                last_info.to.is_dst == info.from.is_dst) {
              // Redundant entry.
              continue;
            }
          }
          info.kind = time_zone::civil_lookup::UNIQUE;
        } else {
          info.kind = time_zone::civil_lookup::REPEATED;
        }
        if (!result.empty()) {
          const auto& last_info = result.back();
          if (last_info.kind == info.kind &&
              last_info.from.civil_time == info.from.civil_time &&
              last_info.from.offset_seconds == info.from.offset_seconds &&
              last_info.from.is_dst == info.from.is_dst &&
              last_info.to.civil_time == info.to.civil_time &&
              last_info.to.offset_seconds == info.to.offset_seconds &&
              last_info.to.is_dst == info.to.is_dst &&
              last_info.tp == info.tp) {
            // Redundant entry.
            continue;
          }
        }
        result.push_back(info);
        last_base_info = transition.to;
      }
    }
    // Remove redundant UNIQUE entries at the beginning.
    while (!result.empty()) {
      const auto& front = result.front();
      if (front.kind != time_zone::civil_lookup::UNIQUE) {
        break;
      }
      result.pop_front();
    }
    return result;
  }

 private:
  TimeZoneRegistry() = delete;
  TimeZoneRegistry(WinTimeZoneRegistryInfo info, AbbreviationMap abbr_map)
      : timezone_info_(std::move(info)), abbr_map_(std::move(abbr_map)) {}

  static bool IsFixedTimeZone(const WinTimeZoneRegistryEntry& entry) {
    return entry.standard_date.month == 0 && entry.daylight_date.month == 0;
  }

  static bool ResolveSystemTime(const WinSystemTime& system_time, year_t year,
                                civil_second* result) {
    const year_t system_time_year = static_cast<year_t>(system_time.year);
    if (system_time_year == year) {
      *result = civil_second(system_time_year, system_time.month,
                             system_time.day, system_time.hour,
                             system_time.minute, system_time.second);
      return true;
    }
    if (system_time_year != 0) {
      return false;
    }

    // Assume the loader has already validated day_of_week to be in [0, 6].
    const cctz::weekday target_weekday = kWeekdays[system_time.day_of_week];
    cctz::civil_day target_day;
    if (system_time.day == 5) {
      // SYSTEMTIME::wDay == 5 means the last weekday of the month.
      year_t tmp_year = year;
      std::int_fast32_t tmp_month = system_time.month + 1;
      if (tmp_month > 12) {
        tmp_month = 1;
        tmp_year += 1;
      }
      target_day =
          prev_weekday(cctz::civil_day(tmp_year, tmp_month, 1), target_weekday);
    } else {
      // Calcurate the first target weekday of the month.
      target_day = next_weekday(cctz::civil_day(year, system_time.month, 1) - 1,
                                target_weekday);
      // Adjust the week number based on the wDay field.
      target_day += (system_time.day - 1) * 7;
    }

    civil_second cs(target_day.year(), target_day.month(), target_day.day(),
                    system_time.hour, system_time.minute, system_time.second);
    // Special rule for "23:59:59.999".
    // https://stackoverflow.com/a/47106207
    if (cs.hour() == 23 && cs.minute() == 59 && cs.second() == 59 &&
        system_time.milliseconds == 999) {
      cs += 1;
    }
    *result = cs;
    return true;
  }

  static std::deque<RawTransitionInfo> ParseTimeZoneInfo(
      const WinTimeZoneRegistryEntry& format, year_t year) {
    const civil_second year_begin(year, 1, 1, 0, 0, 0);
    bool has_std_begin = false;
    civil_second std_begin;
    if (format.standard_date.month != 0) {
      has_std_begin = ResolveSystemTime(format.standard_date, year, &std_begin);
    }
    bool has_dst_begin = false;
    civil_second dst_begin;
    if (format.daylight_date.month != 0) {
      has_dst_begin = ResolveSystemTime(format.daylight_date, year, &dst_begin);
    }

    std::deque<RawTransitionInfo> result;
    if (!(has_std_begin && std_begin == year_begin) &&
        !(has_dst_begin && dst_begin == year_begin)) {
      RawTransitionInfo info;
      info.from_civil_time = year_begin;
      info.to.offset_seconds = -60 * format.bias;
      info.to.dst = false;
      result.push_back(info);
    }
    if (has_std_begin) {
      RawTransitionInfo info;
      info.from_civil_time = std_begin;
      info.to.offset_seconds = -60 * (format.bias + format.standard_bias);
      info.to.dst = false;
      result.push_back(info);
    }
    if (has_dst_begin) {
      RawTransitionInfo info;
      info.from_civil_time = dst_begin;
      info.to.offset_seconds = -60 * (format.bias + format.daylight_bias);
      info.to.dst = true;
      if (has_std_begin) {
        if (dst_begin < std_begin) {
          result.insert(result.end() - 1, info);
        } else if (dst_begin == std_begin) {
          result.pop_back();
          result.push_back(info);
        } else {
          result.push_back(info);
        }
      } else {
        result.push_back(info);
      }
    }

    return result;
  }

  const WinTimeZoneRegistryInfo timezone_info_;
  const AbbreviationMap abbr_map_;
};

class TransitionCache {
 public:
  static TransitionCache Create(const TimeZoneRegistry& timezone_registry) {
    return CreateInternal(timezone_registry);
  }

  bool Hit(const civil_second& cs) const {
    return (transitions_.front().earlier_cs() <= cs &&
            cs <= transitions_.back().later_cs()) ||
           (starts_with_fixed_ && cs < transitions_.front().earlier_cs()) ||
           (ends_with_fixed_ && transitions_.back().later_cs() < cs);
  }
  bool Hit(const time_point<seconds>& tp) const {
    return (transitions_.front().tp <= tp && tp <= transitions_.back().tp) ||
           (starts_with_fixed_ && tp < transitions_.front().tp) ||
           (ends_with_fixed_ && transitions_.back().tp < tp);
  }

  const std::deque<TimeOffsetInfo>& Get() const { return transitions_; }

 private:
  TransitionCache(std::deque<TimeOffsetInfo> transitions,
                  bool starts_with_fixed, bool ends_with_fixed)
      : transitions_(std::move(transitions)),
        starts_with_fixed_(starts_with_fixed),
        ends_with_fixed_(ends_with_fixed) {}

  static TransitionCache CreateInternal(
      const TimeZoneRegistry& timezone_registry) {
    const auto utc_now = civil_second(1970, 1, 1) + std::time(nullptr);

    const year_t utc_year = utc_now.year();
    year_t first_year = utc_year - 16;
    year_t last_year = utc_year + 16;
    bool starts_with_fixed = false;
    bool ends_with_fixed = false;

    if (timezone_registry.IsYearDependent()) {
      starts_with_fixed = timezone_registry.StartsWithFixed();
      ends_with_fixed = timezone_registry.EndsWithFixed();
      if (starts_with_fixed) {
        first_year = timezone_registry.FirstYear();
      } else {
        first_year =
            std::min<year_t>(timezone_registry.FirstYear() - 3, first_year);
      }
      if (ends_with_fixed) {
        last_year = timezone_registry.LastYear() + 1;
      } else {
        last_year =
            std::max<year_t>(timezone_registry.LastYear() + 3, last_year);
      }
    }
    return TransitionCache(
        timezone_registry.GetOffsetInfo(first_year, last_year),
        starts_with_fixed, ends_with_fixed);
  }

  const std::deque<TimeOffsetInfo> transitions_;
  const bool starts_with_fixed_;
  const bool ends_with_fixed_;
};

class DynamicTimeZone final : public TimeZoneIf {
 public:
  static std::unique_ptr<DynamicTimeZone> Create(
      TimeZoneRegistry timezone_registry) {
    auto cache = TransitionCache::Create(timezone_registry);
    return std::unique_ptr<DynamicTimeZone>(
        new DynamicTimeZone(std::move(timezone_registry), std::move(cache)));
  }

  DynamicTimeZone(const DynamicTimeZone&) = delete;
  DynamicTimeZone(DynamicTimeZone&&) = delete;
  DynamicTimeZone& operator=(const DynamicTimeZone&) = delete;

  // TimeZoneIf implementations.
  time_zone::absolute_lookup BreakTime(
      const time_point<seconds>& tp) const override {
    const auto utc = TpToUtc(tp);
    const std::deque<TimeOffsetInfo>& offsets =
        transition_cache_.Hit(tp)
            ? transition_cache_.Get()
            : tz_reg_.GetOffsetInfo(utc.year() - 1, utc.year() + 1);
    if (offsets.empty()) {
      return {};
    }
    const LocalTimeInfo* info = nullptr;
    {
      if (tp < offsets.front().tp) {
        info = &offsets.front().from;
      } else {
        for (size_t i = 1; i < offsets.size(); ++i) {
          if (offsets[i - 1].tp <= tp && tp < offsets[i].tp) {
            info = &offsets[i - 1].to;
            break;
          }
        }
      }
      if (info == nullptr) {
        info = &offsets.back().to;
      }
    }
    const std::int_fast32_t offset_seconds = info->offset_seconds;
    time_zone::absolute_lookup result;
    result.cs = utc + offset_seconds;
    result.offset = offset_seconds;
    result.is_dst = info->is_dst;
    result.abbr = tz_reg_.GetAbbreviation(offset_seconds);
    return result;
  }

  time_zone::civil_lookup MakeTime(const civil_second& cs) const override {
    const auto& offsets =
        transition_cache_.Hit(cs)
            ? transition_cache_.Get()
            : tz_reg_.GetOffsetInfo(cs.year() - 1, cs.year() + 1);
    if (offsets.empty()) {
      return {};
    }

    if (cs < offsets.front().earlier_cs()) {
      time_zone::civil_lookup result;
      result.kind = time_zone::civil_lookup::UNIQUE;
      result.pre = UtcToTp(cs - offsets.front().from.offset_seconds);
      result.post = result.pre;
      result.trans = result.pre;
      return result;
    }

    for (size_t i = 0; i < offsets.size(); ++i) {
      const auto& current = offsets[i];
      if (current.earlier_cs() <= cs && cs < current.later_cs()) {
        time_zone::civil_lookup result;
        result.kind = current.kind;
        result.pre = UtcToTp(cs - current.from.offset_seconds);
        result.post = UtcToTp(cs - current.to.offset_seconds);
        result.trans = current.tp;
        return result;
      }
      if ((i + 1) < offsets.size()) {
        const auto& next = offsets[i + 1];
        if (current.later_cs() <= cs && cs < next.earlier_cs()) {
          time_zone::civil_lookup result;
          result.kind = time_zone::civil_lookup::UNIQUE;
          result.pre = UtcToTp(cs - current.to.offset_seconds);
          result.post = result.pre;
          result.trans = result.pre;
          return result;
        }
      }
    }

    time_zone::civil_lookup result;
    result.kind = time_zone::civil_lookup::UNIQUE;
    result.pre = UtcToTp(cs - offsets.back().to.offset_seconds);
    result.post = result.pre;
    result.trans = result.pre;
    return result;
  }

  bool NextTransition(const time_point<seconds>& tp,
                      time_zone::civil_transition* trans) const override {
    const auto& transitions = transition_cache_.Get();
    if (transitions.empty()) {
      return false;
    }
    const auto it = std::upper_bound(
        transitions.begin(), transitions.end(), tp,
        [](const time_point<seconds>& value, const TimeOffsetInfo& info) {
          return value < info.tp;
        });
    if (it == transitions.end()) {
      return false;
    }
    trans->from = it->from.civil_time;
    trans->to = it->to.civil_time;
    return true;
  }

  bool PrevTransition(const time_point<seconds>& tp,
                      time_zone::civil_transition* trans) const override {
    const auto& transitions = transition_cache_.Get();
    if (transitions.empty()) {
      return false;
    }
    auto it = std::lower_bound(
        transitions.begin(), transitions.end(), tp,
        [](const TimeOffsetInfo& info, const time_point<seconds>& value) {
          return info.tp < value;
        });
    if (it == transitions.begin()) {
      return false;
    }
    --it;
    trans->from = it->from.civil_time;
    trans->to = it->to.civil_time;
    return true;
  }

  std::string Version() const override { return std::string(); }

  std::string Description() const override { return std::string(); }

 private:
  DynamicTimeZone(TimeZoneRegistry timezone_map,
                  TransitionCache transition_cache)
      : tz_reg_(std::move(timezone_map)),
        transition_cache_(std::move(transition_cache)) {}

  const TimeZoneRegistry tz_reg_;
  const TransitionCache transition_cache_;
};

class FixedTimeZone final : public TimeZoneIf {
 public:
  static std::unique_ptr<FixedTimeZone> Create(std::int_fast32_t offset_sec) {
    const char* common_abbr = GetCommonAbbreviation(offset_sec);
    return std::unique_ptr<FixedTimeZone>(new FixedTimeZone(
        offset_sec,
        common_abbr != nullptr
            ? common_abbr
            : "GMT" + cctz::FixedOffsetToAbbr(cctz::seconds(offset_sec))));
  }

  FixedTimeZone(const FixedTimeZone&) = delete;
  FixedTimeZone(FixedTimeZone&&) = delete;
  FixedTimeZone& operator=(const FixedTimeZone&) = delete;

  time_zone::absolute_lookup BreakTime(
      const time_point<seconds>& tp) const override {
    time_zone::absolute_lookup result;
    result.cs = TpToUtc(tp) + offset_sec_;
    result.offset = offset_sec_;
    result.is_dst = false;
    result.abbr = abbr_.c_str();
    return result;
  }

  time_zone::civil_lookup MakeTime(const civil_second& cs) const override {
    time_zone::civil_lookup result;
    result.kind = time_zone::civil_lookup::UNIQUE;
    result.pre = UtcToTp(cs - offset_sec_);
    result.post = result.pre;
    result.trans = result.pre;
    return result;
  }

  bool NextTransition(const time_point<seconds>& tp,
                      time_zone::civil_transition* trans) const override {
    return false;
  }

  bool PrevTransition(const time_point<seconds>& tp,
                      time_zone::civil_transition* trans) const override {
    return false;
  }

  std::string Version() const override { return std::string(); }
  std::string Description() const override { return std::string(); }

 private:
  FixedTimeZone(std::int_fast32_t offset_sec, std::string abbr)
      : offset_sec_(offset_sec), abbr_(std::move(abbr)) {}

  const std::int_fast32_t offset_sec_;
  const std::string abbr_;
};

}  // namespace

std::unique_ptr<TimeZoneIf> MakeTimeZoneFromWinRegistry(
    WinTimeZoneRegistryInfo info) {
  if (info.entries.empty()) {
    return nullptr;
  }
  auto timezone_registry = TimeZoneRegistry::Load(std::move(info));

  if (timezone_registry.IsFixed()) {
    const std::int_fast32_t offset_seconds = timezone_registry.GetFixedOffset();
    return FixedTimeZone::Create(offset_seconds);
  }

  return DynamicTimeZone::Create(std::move(timezone_registry));
}

}  // namespace cctz
