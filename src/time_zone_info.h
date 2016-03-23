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

#ifndef CCTZ_TIME_ZONE_INFO_H_
#define CCTZ_TIME_ZONE_INFO_H_

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "time_zone_if.h"
#include "tzfile.h"

namespace cctz {

// A zone-independent date/time. A DateTime represents a "Y/M/D H:M:S"
// as an offset in seconds from some epoch DateTime, without taking into
// account the value of, or changes in any time_zone's UTC offset (i.e., as
// if the date/time was in UTC). This allows "Y/M/D H:M:S" values to be
// quickly ordered by offset (although this may not be the same ordering as
// their corresponding times in a time_zone). Also, if two DateTimes are not
// separated by a UTC-offset change in some time_zone, then the number of
// seconds between them can be computed as a simple difference of offsets.
//
// Note: Because the DateTime epoch does not correspond to the time_point
// epoch (even if only because of the unknown UTC offset) there can be valid
// times that will not be representable as DateTimes when DateTime only has
// the same number of "seconds" bits. We accept this at the moment (so as
// to avoid extended arithmetic) and lose a little range as a result.
struct DateTime {
  int64_t offset;  // seconds from some epoch DateTime
  bool Normalize(int64_t year, int mon, int day, int hour, int min, int sec);
  void Assign(const Breakdown& bd);
};

inline bool operator<(const DateTime& lhs, const DateTime& rhs) {
  return lhs.offset < rhs.offset;
}

// The difference between two DateTimes in seconds. Requires that all
// intervening DateTimes share the same UTC offset (i.e., no transitions).
inline int64_t operator-(const DateTime& lhs, const DateTime& rhs) {
  return lhs.offset - rhs.offset;
}

// A transition to a new UTC offset.
struct Transition {
  int64_t unix_time;        // the instant of this transition
  uint8_t type_index;       // index of the transition type
  DateTime date_time;       // local date/time of transition
  DateTime prev_date_time;  // local date/time one second earlier

  struct ByUnixTime {
    inline bool operator()(const Transition& lhs, const Transition& rhs) const {
      return lhs.unix_time < rhs.unix_time;
    }
  };
  struct ByDateTime {
    inline bool operator()(const Transition& lhs, const Transition& rhs) const {
      return lhs.date_time < rhs.date_time;
    }
  };
};

// The characteristics of a particular transition.
struct TransitionType {
  int32_t utc_offset;  // the new prevailing UTC offset
  bool is_dst;         // did we move into daylight-saving time
  uint8_t abbr_index;  // index of the new abbreviation
};


// A time zone backed by the IANA Time Zone Database (zoneinfo).
class TimeZoneInfo : public TimeZoneIf {
 public:
  TimeZoneInfo() = default;
  TimeZoneInfo(const TimeZoneInfo&) = delete;
  TimeZoneInfo& operator=(const TimeZoneInfo&) = delete;

  // Loads the zoneinfo for the given name, returning true if successful.
  bool Load(const std::string& name);

  // TimeZoneIf implementations.
  Breakdown BreakTime(const time_point<sys_seconds>& tp) const override;
  TimeInfo MakeTimeInfo(int64_t year, int mon, int day,
                        int hour, int min, int sec) const override;

 private:
  struct Header {  // counts of:
    int32_t timecnt;     // transition times
    int32_t typecnt;     // transition types
    int32_t charcnt;     // zone abbreviation characters
    int32_t leapcnt;     // leap seconds (we expect none)
    int32_t ttisstdcnt;  // UTC/local indicators (unused)
    int32_t ttisgmtcnt;  // standard/wall indicators (unused)

    void Build(const tzhead& tzh);
    size_t DataLength(size_t time_len) const;
  };

  void CheckTransition(const std::string& name, const TransitionType& tt,
                       int32_t offset, bool is_dst,
                       const std::string& abbr) const;

  void ResetToBuiltinUTC(int seconds);
  bool Load(const std::string& name, FILE* fp);

  // Helpers for BreakTime() and MakeTimeInfo() respectively.
  Breakdown LocalTime(int64_t unix_time, const TransitionType& tt) const;
  TimeInfo TimeLocal(int64_t year, int mon, int day,
                     int hour, int min, int sec, int64_t offset) const;

  std::vector<Transition> transitions_;  // ordered by unix_time and date_time
  std::vector<TransitionType> transition_types_;  // distinct transition types
  int default_transition_type_;  // for before the first transition
  std::string abbreviations_;  // all the NUL-terminated abbreviations

  std::string future_spec_;  // for after the last zic transition
  bool extended_;            // future_spec_ was used to generate transitions
  int64_t last_year_;        // the final year of the generated transitions

  mutable std::atomic<size_t> local_time_hint_;  // BreakTime() search hint
  mutable std::atomic<size_t> time_local_hint_;  // MakeTimeInfo() search hint
};

}  // namespace cctz

#endif  // CCTZ_TIME_ZONE_INFO_H_
