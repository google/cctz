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

// This file implements the TimeZoneIf interface using the "zoneinfo"
// data provided by the IANA Time Zone Database (i.e., the only real game
// in town).
//
// TimeZoneInfo represents the history of UTC-offset changes within a time
// zone. Most changes are due to daylight-saving rules, but occasionally
// shifts are made to the time-zone's base offset. The database only attempts
// to be definitive for times since 1970, so be wary of local-time conversions
// before that. Also, rule and zone-boundary changes are made at the whim
// of governments, so the conversion of future times needs to be taken with
// a grain of salt.
//
// For more information see tzfile(5), http://www.iana.org/time-zones, or
// http://en.wikipedia.org/wiki/Zoneinfo.
//
// Note that we assume the proleptic Gregorian calendar and 60-second
// minutes throughout.

#include "src/cctz_info.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>

#include "src/cctz_posix.h"

namespace cctz {

namespace {

// Wrap the tzfile.h isleap() macro with an inline function, which will
// then have normal argument-passing semantics (i.e., single evaluation).
inline bool IsLeap(int64_t year) { return isleap(year); }

// The month lengths in non-leap and leap years respectively.
const int8_t kDaysPerMonth[2][1 + MONSPERYEAR] = {
  {-1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {-1, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

// The day offsets of the beginning of each (1-based) month in non-leap
// and leap years respectively. That is, sigma[1:n]:kDaysPerMonth[][i].
// For example, in a leap year there are 335 days before December.
const int16_t kMonthOffsets[2][1 + MONSPERYEAR + 1] = {
  {-1, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  {-1, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
};

// 400-year chunks always have 146097 days (20871 weeks).
const int64_t kSecPer400Years = 146097LL * SECSPERDAY;

// The number of seconds in an aligned 100-year chunk, for those that
// do not begin with a leap year and those that do respectively.
const int64_t kSecPer100Years[2] = {
  (76LL * DAYSPERNYEAR + 24LL * DAYSPERLYEAR) * SECSPERDAY,
  (75LL * DAYSPERNYEAR + 25LL * DAYSPERLYEAR) * SECSPERDAY,
};

// The number of seconds in an aligned 4-year chunk, for those that
// do not begin with a leap year and those that do respectively.
const int32_t kSecPer4Years[2] = {
  (4 * DAYSPERNYEAR + 0 * DAYSPERLYEAR) * SECSPERDAY,
  (3 * DAYSPERNYEAR + 1 * DAYSPERLYEAR) * SECSPERDAY,
};

// The number of seconds in non-leap and leap years respectively.
const int32_t kSecPerYear[2] = {
  DAYSPERNYEAR * SECSPERDAY,
  DAYSPERLYEAR * SECSPERDAY,
};

// The number of days in the 100 years starting in the mod-400 index year,
// stored as a 36524-deficit value (i.e., 0 == 36524, 1 == 36525).
const int8_t kDaysPer100Years[401] = {
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

inline int DaysPer100Years(int eyear) {
  return 36524 + kDaysPer100Years[eyear];
}

// The number of days in the 4 years starting in the mod-400 index year,
// stored as a 1460-deficit value (i.e., 0 == 1460, 1 == 1461).
const int8_t kDaysPer4Years[401] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

inline int DaysPer4Years(int eyear) { return 1460 + kDaysPer4Years[eyear]; }

// Like kSecPerYear[] but scaled down by a factor of SECSPERDAY.
const int32_t kDaysPerYear[2] = {DAYSPERNYEAR, DAYSPERLYEAR};

inline int DaysPerYear(int year) { return kDaysPerYear[IsLeap(year)]; }

// Map a (normalized) Y/M/D to the number of days before/after 1970-01-01.
// See http://howardhinnant.github.io/date_algorithms.html#days_from_civil.
int64_t DayOrdinal(int64_t year, int month, int day) {
  year -= (month <= 2 ? 1 : 0);
  const int64_t era = (year >= 0 ? year : year - 399) / 400;
  const int yoe = year - era * 400;
  const int doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;  // shift epoch to 1970-01-01
}

// Normalize (*valp + carry_in) so that [zero <= *valp < (zero + base)],
// returning multiples of base to carry out. "zero" must be >= 0, and
// base must be sufficiently large to avoid overflowing the return value.
// Inlining admits significant gains as base and zero are literals.
inline int NormalizeField(int base, int zero, int* valp, int carry_in) {
  int carry_out = 0;
  int val = *valp;
  if (zero != 0 && val < 0) {
    val += base;
    carry_out -= 1;
  }
  val -= zero;
  carry_out += val / base;
  int rem = val % base;
  if (carry_in != 0) {
    carry_out += carry_in / base;
    rem += carry_in % base;
    if (rem < 0) {
      carry_out -= 1;
      rem += base;
    } else if (rem >= base) {
      carry_out += 1;
      rem -= base;
    }
  }
  if (rem < 0) {
    carry_out -= 1;
    rem += base;
  }
  *valp = rem + zero;
  return carry_out;
}

// Multi-byte, numeric values are encoded using a MSB first,
// twos-complement representation. These helpers decode, from
// the given address, 4-byte and 8-byte values respectively.
int32_t Decode32(const char* cp) {
  uint32_t v = 0;
  for (int i = 0; i != (32 / 8); ++i)
    v = (v << 8) | (static_cast<uint8_t>(*cp++) & 0xff);
  if ((v & ~(UINT32_MAX >> 1)) == 0) return v;
  return -static_cast<int32_t>(~v) - 1;
}

int64_t Decode64(const char* cp) {
  uint64_t v = 0;
  for (int i = 0; i != (64 / 8); ++i)
    v = (v << 8) | (static_cast<uint8_t>(*cp++) & 0xff);
  if ((v & ~(UINT64_MAX >> 1)) == 0) return v;
  return -static_cast<int64_t>(~v) - 1;
}

// Generate a year-relative offset for a PosixTransition.
int64_t TransOffset(bool leap_year, int jan1_weekday,
                    const PosixTransition& pt) {
  const bool last_week = (pt.week == 5);
  int days = kMonthOffsets[leap_year][pt.month + last_week];
  const int weekday = (jan1_weekday + days) % DAYSPERWEEK;
  if (last_week) {
    days -= (weekday + DAYSPERWEEK - 1 - pt.weekday) % DAYSPERWEEK + 1;
  } else {
    days += (pt.weekday + DAYSPERWEEK - weekday) % DAYSPERWEEK;
    days += (pt.week - 1) * DAYSPERWEEK;
  }
  return (days * SECSPERDAY) + pt.offset;
}

inline TimeInfo MakeUnique(int64_t unix_time, bool normalized) {
  TimeInfo ti;
  ti.pre = ti.trans = ti.post = FromUnixSeconds(unix_time);
  ti.kind = TimeInfo::Kind::UNIQUE;
  ti.normalized = normalized;
  return ti;
}

inline TimeInfo MakeSkipped(const Transition& tr, const DateTime& dt,
                            bool normalized) {
  TimeInfo ti;
  ti.pre = FromUnixSeconds(tr.unix_time - 1 + (dt - tr.prev_date_time));
  ti.trans = FromUnixSeconds(tr.unix_time);
  ti.post = FromUnixSeconds(tr.unix_time - (tr.date_time - dt));
  ti.kind = TimeInfo::Kind::SKIPPED;
  ti.normalized = normalized;
  return ti;
}

inline TimeInfo MakeRepeated(const Transition& tr, const DateTime& dt,
                             bool normalized) {
  TimeInfo ti;
  ti.pre = FromUnixSeconds(tr.unix_time - 1 - (tr.prev_date_time - dt));
  ti.trans = FromUnixSeconds(tr.unix_time);
  ti.post = FromUnixSeconds(tr.unix_time + (dt - tr.date_time));
  ti.kind = TimeInfo::Kind::REPEATED;
  ti.normalized = normalized;
  return ti;
}

}  // namespace

// Normalize from individual date/time fields.
bool DateTime::Normalize(int64_t year, int mon, int day,
                         int hour, int min, int sec) {
  int min_carry = NormalizeField(SECSPERMIN, 0, &sec, 0);
  int hour_carry = NormalizeField(MINSPERHOUR, 0, &min, min_carry);
  int day_carry = NormalizeField(HOURSPERDAY, 0, &hour, hour_carry);
  int year_carry = NormalizeField(MONSPERYEAR, 1, &mon, 0);
  bool normalized = min_carry || hour_carry || day_carry || year_carry;

  // Normalize the number of days within a 400-year (146097-day) period.
  if (int c4_carry = NormalizeField(146097, 1, &day, day_carry)) {
    year_carry += c4_carry * 400;
    normalized = true;
  }

  // Extract a [0:399] year calendrically equivalent to (year + year_carry)
  // from that sum in order to simplify year/day normalization and to defer
  // the possibility of int64_t overflow until the final stage.
  int eyear = year % 400;
  if (year_carry != 0) { eyear += year_carry; eyear %= 400; }
  if (eyear < 0) eyear += 400;
  year_carry -= eyear;

  int orig_day = day;
  if (day > DAYSPERNYEAR) {
    eyear += (mon > 2 ? 1 : 0);
    if (day > 146097 - DAYSPERNYEAR) {
      // We often hit the 400th year when stepping a civil time backwards,
      // so special case it to avoid counting up by 100/4/1 year chunks.
      day = DaysPerYear(eyear += 400 - 1) - (146097 - day);
    } else {
      // Handle days in chunks of 100/4/1 years.
      for (int ydays = DaysPer100Years(eyear); day > ydays;
           day -= ydays, ydays = DaysPer100Years(eyear)) {
        if ((eyear += 100) > 400) { eyear -= 400; year_carry += 400; }
      }
      for (int ydays = DaysPer4Years(eyear); day > ydays;
           day -= ydays, ydays = DaysPer4Years(eyear)) {
        if ((eyear += 4) > 400) { eyear -= 400; year_carry += 400; }
      }
      for (int ydays = DaysPerYear(eyear); day > ydays;
           day -= ydays, ydays = DaysPerYear(eyear)) {
        eyear += 1;
      }
    }
    eyear -= (mon > 2 ? 1 : 0);
  }
  // Handle days within one year.
  bool leap_year = IsLeap(eyear);
  for (int mdays = kDaysPerMonth[leap_year][mon]; day > mdays;
       day -= mdays, mdays = kDaysPerMonth[leap_year][mon]) {
    if (++mon > MONSPERYEAR) { mon = 1; leap_year = IsLeap(++eyear); }
  }
  if (day != orig_day) normalized = true;

  // Add the updated eyear back into (year + year_carry).
  year_carry += eyear;

  // Finally, set the DateTime offset.
  offset = DayOrdinal(year + year_carry, mon, day);
  if (offset < 0) {
    offset += 1;
    offset *= SECSPERHOUR * HOURSPERDAY;
    offset += hour * SECSPERHOUR + min * SECSPERMIN + sec;
    offset -= SECSPERHOUR * HOURSPERDAY;
  } else {
    offset *= SECSPERHOUR * HOURSPERDAY;
    offset += hour * SECSPERHOUR + min * SECSPERMIN + sec;
  }
  return normalized;
}

// Assign from a Breakdown, created using a TimeZoneInfo timestamp.
inline void DateTime::Assign(const Breakdown& bd) {
  Normalize(bd.year, bd.month, bd.day, bd.hour, bd.minute, bd.second);
}

// What (no leap-seconds) UTC+seconds zoneinfo would look like.
void TimeZoneInfo::ResetToBuiltinUTC(int seconds) {
  transition_types_.resize(1);
  transition_types_[0].utc_offset = seconds;
  transition_types_[0].is_dst = false;
  transition_types_[0].abbr_index = 0;
  transitions_.resize(1);
  transitions_[0].unix_time = -(1LL << 59);  // zic "BIG_BANG"
  transitions_[0].type_index = 0;
  transitions_[0].date_time.Assign(
      LocalTime(transitions_[0].unix_time, transition_types_[0]));
  transitions_[0].prev_date_time = transitions_[0].date_time;
  transitions_[0].prev_date_time.offset -= 1;
  default_transition_type_ = 0;
  abbreviations_ = "UTC";  // TODO: handle non-zero offset
  abbreviations_.append(1, '\0');  // add NUL
  future_spec_.clear();  // never needed for a fixed-offset zone
  extended_ = false;
}

// Builds the in-memory header using the raw bytes from the file.
void TimeZoneInfo::Header::Build(const tzhead& tzh) {
  timecnt = Decode32(tzh.tzh_timecnt);
  typecnt = Decode32(tzh.tzh_typecnt);
  charcnt = Decode32(tzh.tzh_charcnt);
  leapcnt = Decode32(tzh.tzh_leapcnt);
  ttisstdcnt = Decode32(tzh.tzh_ttisstdcnt);
  ttisgmtcnt = Decode32(tzh.tzh_ttisgmtcnt);
}

void TimeZoneInfo::CheckTransition(const std::string& name,
                                   const TransitionType& tt, int32_t offset,
                                   bool is_dst, const std::string& abbr) const {
  if (tt.utc_offset != offset || tt.is_dst != is_dst ||
      &abbreviations_[tt.abbr_index] != abbr) {
    std::clog << name << ": Transition"
              << " offset=" << tt.utc_offset << "/"
              << (tt.is_dst ? "DST" : "STD")
              << "/abbr=" << &abbreviations_[tt.abbr_index]
              << " does not match POSIX spec '" << future_spec_ << "'\n";
  }
}

// How many bytes of data are associated with this header. The result
// depends upon whether this is a section with 4-byte or 8-byte times.
size_t TimeZoneInfo::Header::DataLength(size_t time_len) const {
  size_t len = 0;
  len += (time_len + 1) * timecnt;  // unix_time + type_index
  len += (4 + 1 + 1) * typecnt;     // utc_offset + is_dst + abbr_index
  len += 1 * charcnt;               // abbreviations
  len += (time_len + 4) * leapcnt;  // leap-time + TAI-UTC
  len += 1 * ttisstdcnt;            // UTC/local indicators
  len += 1 * ttisgmtcnt;            // standard/wall indicators
  return len;
}

bool TimeZoneInfo::Load(const std::string& name, FILE* fp) {
  // Read and validate the header.
  tzhead tzh;
  if (fread(&tzh, 1, sizeof tzh, fp) != sizeof tzh)
    return false;
  if (strncmp(tzh.tzh_magic, TZ_MAGIC, sizeof(tzh.tzh_magic)) != 0)
    return false;
  Header hdr;
  hdr.Build(tzh);
  size_t time_len = 4;
  if (tzh.tzh_version[0] != '\0') {
    // Skip the 4-byte data.
    if (fseek(fp, hdr.DataLength(time_len), SEEK_CUR) != 0)
      return false;
    // Read and validate the header for the 8-byte data.
    if (fread(&tzh, 1, sizeof tzh, fp) != sizeof tzh)
      return false;
    if (strncmp(tzh.tzh_magic, TZ_MAGIC, sizeof(tzh.tzh_magic)) != 0)
      return false;
    if (tzh.tzh_version[0] == '\0')
      return false;
    hdr.Build(tzh);
    time_len = 8;
  }
  if (hdr.timecnt < 0 || hdr.typecnt <= 0)
    return false;
  if (hdr.leapcnt != 0) {
    // This code assumes 60-second minutes so we do not want
    // the leap-second encoded zoneinfo. We could reverse the
    // compensation, but it's never in a Google zoneinfo anyway,
    // so currently we simply reject such data.
    return false;
  }
  if (hdr.ttisstdcnt != 0 && hdr.ttisstdcnt != hdr.typecnt)
    return false;
  if (hdr.ttisgmtcnt != 0 && hdr.ttisgmtcnt != hdr.typecnt)
    return false;

  // Read the data into a local buffer.
  size_t len = hdr.DataLength(time_len);
  std::vector<char> tbuf(len);
  if (fread(tbuf.data(), 1, len, fp) != len)
    return false;
  const char* bp = tbuf.data();

  // Decode and validate the transitions.
  transitions_.resize(hdr.timecnt);
  for (int32_t i = 0; i != hdr.timecnt; ++i) {
    transitions_[i].unix_time = (time_len == 4) ? Decode32(bp) : Decode64(bp);
    bp += time_len;
    if (i != 0) {
      // Check that the transitions are ordered by time (as zic guarantees).
      if (!Transition::ByUnixTime()(transitions_[i - 1], transitions_[i]))
        return false;  // out of order
    }
  }
  bool seen_type_0 = false;
  for (int32_t i = 0; i != hdr.timecnt; ++i) {
    transitions_[i].type_index = (static_cast<uint8_t>(*bp++) & 0xff);
    if (transitions_[i].type_index >= hdr.typecnt)
      return false;
    if (transitions_[i].type_index == 0)
      seen_type_0 = true;
  }

  // Decode and validate the transition types.
  transition_types_.resize(hdr.typecnt);
  for (int32_t i = 0; i != hdr.typecnt; ++i) {
    transition_types_[i].utc_offset = Decode32(bp);
    if (transition_types_[i].utc_offset >= SECSPERDAY ||
        transition_types_[i].utc_offset <= -SECSPERDAY)
      return false;
    bp += 4;
    transition_types_[i].is_dst = ((static_cast<uint8_t>(*bp++) & 0xff) != 0);
    transition_types_[i].abbr_index = (static_cast<uint8_t>(*bp++) & 0xff);
    if (transition_types_[i].abbr_index >= hdr.charcnt)
      return false;
  }

  // Determine the before-first-transition type.
  default_transition_type_ = 0;
  if (seen_type_0 && hdr.timecnt != 0) {
    uint8_t index = 0;
    if (transition_types_[0].is_dst) {
      index = transitions_[0].type_index;
      while (index != 0 && transition_types_[index].is_dst)
        --index;
    }
    while (index != hdr.typecnt && transition_types_[index].is_dst)
      ++index;
    if (index != hdr.typecnt)
      default_transition_type_ = index;
  }

  // Copy all the abbreviations.
  abbreviations_.assign(bp, hdr.charcnt);
  bp += hdr.charcnt;

  // Skip the unused portions. We've already dispensed with leap-second
  // encoded zoneinfo. The ttisstd/ttisgmt indicators only apply when
  // interpreting a POSIX spec that does not include start/end rules, and
  // that isn't the case here (see "zic -p").
  bp += (8 + 4) * hdr.leapcnt;  // leap-time + TAI-UTC
  bp += 1 * hdr.ttisstdcnt;     // UTC/local indicators
  bp += 1 * hdr.ttisgmtcnt;     // standard/wall indicators

  future_spec_.clear();
  if (tzh.tzh_version[0] != '\0') {
    // Snarf up the NL-enclosed future POSIX spec. Note
    // that version '3' files utilize an extended format.
    if (fgetc(fp) != '\n')
      return false;
    for (int c = fgetc(fp); c != '\n'; c = fgetc(fp)) {
      if (c == EOF)
        return false;
      future_spec_.push_back(c);
    }
  }

  // We don't check for EOF so that we're forwards compatible.

  // Use the POSIX-TZ-environment-variable-style string to handle times
  // in years after the last transition stored in the zoneinfo data.
  extended_ = false;
  if (!future_spec_.empty()) {
    PosixTimeZone posix;
    if (!ParsePosixSpec(future_spec_, &posix)) {
      std::clog << name << ": Failed to parse '" << future_spec_ << "'\n";
    } else if (posix.dst_abbr.empty()) {  // std only
      // The future specification should match the last/default transition,
      // and that means that handling the future will fall out naturally.
      int index = default_transition_type_;
      if (hdr.timecnt != 0) index = transitions_[hdr.timecnt - 1].type_index;
      const TransitionType& tt(transition_types_[index]);
      CheckTransition(name, tt, posix.std_offset, false, posix.std_abbr);
    } else if (hdr.timecnt < 2) {
      std::clog << name << ": Too few transitions for POSIX spec\n";
    } else if (transitions_[hdr.timecnt - 1].unix_time < 0) {
      std::clog << name << ": Old transitions for POSIX spec\n";
    } else {  // std and dst
      // Extend the transitions for an additional 400 years using the
      // future specification. Years beyond those can be handled by
      // mapping back to a cycle-equivalent year within that range.
      // zic(8) should probably do this so that we don't have to.
      transitions_.resize(hdr.timecnt + 400 * 2);
      extended_ = true;

      // The future specification should match the last two transitions,
      // and those transitions should have different is_dst flags but be
      // in the same calendar year.
      const Transition& tr0(transitions_[hdr.timecnt - 1]);
      const Transition& tr1(transitions_[hdr.timecnt - 2]);
      const TransitionType& tt0(transition_types_[tr0.type_index]);
      const TransitionType& tt1(transition_types_[tr1.type_index]);
      const TransitionType& spring(tt0.is_dst ? tt0 : tt1);
      const TransitionType& autumn(tt0.is_dst ? tt1 : tt0);
      CheckTransition(name, spring, posix.dst_offset, true, posix.dst_abbr);
      CheckTransition(name, autumn, posix.std_offset, false, posix.std_abbr);
      last_year_ = LocalTime(tr0.unix_time, tt0).year;
      if (LocalTime(tr1.unix_time, tt1).year != last_year_) {
        std::clog << name << ": Final transitions not in same year\n";
      }

      // Add the transitions to tr1 and back to tr0 for each extra year.
      const PosixTransition& pt1(tt0.is_dst ? posix.dst_end : posix.dst_start);
      const PosixTransition& pt0(tt0.is_dst ? posix.dst_start : posix.dst_end);
      Transition* tr = &transitions_[hdr.timecnt];  // next trans to fill
      const int64_t jan1_ord = DayOrdinal(last_year_, 1, 1);
      int64_t jan1_time = jan1_ord * SECSPERDAY;
      int jan1_weekday = (EPOCH_WDAY + jan1_ord) % DAYSPERWEEK;
      if (jan1_weekday < 0) jan1_weekday += DAYSPERWEEK;
      bool leap_year = IsLeap(last_year_);
      for (const int64_t limit = last_year_ + 400; last_year_ < limit;) {
        last_year_ += 1;  // an additional year of generated transitions
        jan1_time += kSecPerYear[leap_year];
        jan1_weekday = (jan1_weekday + kDaysPerYear[leap_year]) % DAYSPERWEEK;
        leap_year = !leap_year && IsLeap(last_year_);
        const int64_t tr1_offset = TransOffset(leap_year, jan1_weekday, pt1);
        tr->unix_time = jan1_time + tr1_offset - tt0.utc_offset;
        tr++->type_index = tr1.type_index;
        const int64_t tr0_offset = TransOffset(leap_year, jan1_weekday, pt0);
        tr->unix_time = jan1_time + tr0_offset - tt1.utc_offset;
        tr++->type_index = tr0.type_index;
      }
    }
  }

  // Compute the local civil time for each transition and the preceeding
  // second. These will be used for reverse conversions in MakeTimeInfo().
  const TransitionType* ttp = &transition_types_[default_transition_type_];
  for (size_t i = 0; i != transitions_.size(); ++i) {
    Transition& tr(transitions_[i]);
    tr.prev_date_time.Assign(LocalTime(tr.unix_time, *ttp));
    tr.prev_date_time.offset -= 1;
    ttp = &transition_types_[tr.type_index];
    tr.date_time.Assign(LocalTime(tr.unix_time, *ttp));
    if (i != 0) {
      // Check that the transitions are ordered by date/time. Essentially
      // this means that an offset change cannot cross another such change.
      // No one does this in practice, and we depend on it in MakeTimeInfo().
      if (!Transition::ByDateTime()(transitions_[i - 1], tr))
        return false;  // out of order
    }
  }

  // We remember the transitions found during the last BreakTime() and
  // MakeTimeInfo() calls. If the next request is for the same transition
  // we will avoid re-searching.
  local_time_hint_ = 0;
  time_local_hint_ = 0;

  return true;
}

bool TimeZoneInfo::Load(const std::string& name) {
  // We can ensure that the loading of UTC or any other fixed-offset
  // zone never fails because the simple, no-transition state can be
  // internally generated. Note that this depends on our choice to not
  // accept leap-second encoded ("right") zoneinfo.
  if (name == "UTC") {
    ResetToBuiltinUTC(0);
    return true;
  }

  // Map time-zone name to its machine-specific path.
  std::string path;
  if (name == "localtime") {
    const char* localtime = std::getenv("LOCALTIME");
    path = localtime ? localtime : "/etc/localtime";
  } else if (!name.empty() && name[0] == '/') {
    path = name;
  } else {
    const char* tzdir = std::getenv("TZDIR");
    path = tzdir ? tzdir : "/usr/share/zoneinfo";
    path += '/';
    path += name;
  }

  // Load the time-zone data.
  bool loaded = false;
  if (FILE* fp = fopen(path.c_str(), "r")) {
    loaded = Load(name, fp);
    fclose(fp);
  } else {
    char ebuf[64] = "Failed to open";
    strerror_r(errno, ebuf, sizeof ebuf);
    std::clog << path << ": " << ebuf << "\n";
  }
  return loaded;
}

// BreakTime() translation for a particular transition type.
Breakdown TimeZoneInfo::LocalTime(int64_t unix_time,
                                  const TransitionType& tt) const {
  Breakdown bd;

  bd.year = EPOCH_YEAR;
  bd.weekday = EPOCH_WDAY;  // Thu
  int64_t seconds = unix_time;

  // Shift to a base year that is 400-year aligned.
  static_assert(EPOCH_YEAR == 1970, "Unexpected value for EPOCH_YEAR");
  if (seconds >= 0) {
    seconds -= 10957LL * SECSPERDAY;
    bd.year += 30;  // == 2000
  } else {
    seconds += (146097LL - 10957LL) * SECSPERDAY;
    bd.year -= 370;  // == 1600
  }
  bd.weekday += 2;  // Sat

  // A civil time in "+offset" looks like (time+offset) in UTC.
  if (seconds >= 0) {
    if (tt.utc_offset > 0 && seconds > INT64_MAX - tt.utc_offset) {
      seconds -= kSecPer400Years;
      bd.year += 400;
    }
  } else {
    if (tt.utc_offset < 0 && seconds < INT64_MIN - tt.utc_offset) {
      seconds += kSecPer400Years;
      bd.year -= 400;
    }
  }
  seconds += tt.utc_offset;

  // Handle years in chunks of 400/100/4/1.
  bd.year += 400 * (seconds / kSecPer400Years);
  seconds %= kSecPer400Years;
  if (seconds < 0) {
    seconds += kSecPer400Years;
    bd.year -= 400;
  }
  bool leap_year = true;  // 4-century aligned
  int64_t sec_per_100years = kSecPer100Years[leap_year];
  while (seconds >= sec_per_100years) {
    seconds -= sec_per_100years;
    bd.year += 100;
    bd.weekday += 5 + leap_year;
    leap_year = false;  // 1-century, non 4-century aligned
    sec_per_100years = kSecPer100Years[leap_year];
  }
  int32_t sec_per_4years = kSecPer4Years[leap_year];
  while (seconds >= sec_per_4years) {
    seconds -= sec_per_4years;
    bd.year += 4;
    bd.weekday += 4 + leap_year;
    leap_year = true;  // 4-year, non century aligned
    sec_per_4years = kSecPer4Years[leap_year];
  }
  int32_t sec_per_year = kSecPerYear[leap_year];
  while (seconds >= sec_per_year) {
    seconds -= sec_per_year;
    bd.year += 1;
    bd.weekday += 1 + leap_year;
    leap_year = false;  // non 4-year aligned
    sec_per_year = kSecPerYear[leap_year];
  }

  // Handle months and days.
  bd.yearday = (seconds / SECSPERDAY) + 1;
  seconds %= SECSPERDAY;
  bd.month = TM_DECEMBER + 1;
  bd.day = bd.yearday;
  bd.weekday += bd.day - 1;
  while (bd.month != TM_JANUARY + 1) {
    int month_offset = kMonthOffsets[leap_year][bd.month];
    if (bd.day > month_offset) {
      bd.day -= month_offset;
      break;
    }
    bd.month -= 1;
  }

  // Handle hours, minutes, and seconds.
  bd.hour = seconds / SECSPERHOUR;
  seconds %= SECSPERHOUR;
  bd.minute = seconds / SECSPERMIN;
  bd.second = seconds % SECSPERMIN;

  // Shift weekday to [1==Mon, ..., 7=Sun].
  bd.weekday -= 1;
  bd.weekday %= DAYSPERWEEK;
  bd.weekday += 1;

  // Handle offset, is_dst, and abbreviation.
  bd.offset = tt.utc_offset;
  bd.is_dst = tt.is_dst;
  bd.abbr = &abbreviations_[tt.abbr_index];

  return bd;
}

// MakeTimeInfo() translation with a conversion-preserving offset.
TimeInfo TimeZoneInfo::TimeLocal(int64_t year, int mon, int day, int hour,
                                 int min, int sec, int64_t offset) const {
  TimeInfo ti = MakeTimeInfo(year, mon, day, hour, min, sec);
  ti.pre = FromUnixSeconds(ToUnixSeconds(ti.pre) + offset);
  ti.trans = FromUnixSeconds(ToUnixSeconds(ti.trans) + offset);
  ti.post = FromUnixSeconds(ToUnixSeconds(ti.post) + offset);
  return ti;
}

Breakdown TimeZoneInfo::BreakTime(const time_point<seconds64>& tp) const {
  int64_t unix_time = ToUnixSeconds(tp);
  const int32_t timecnt = transitions_.size();
  if (timecnt == 0 || unix_time < transitions_[0].unix_time) {
    const int type_index = default_transition_type_;
    return LocalTime(unix_time, transition_types_[type_index]);
  }
  if (unix_time >= transitions_[timecnt - 1].unix_time) {
    // After the last transition. If we extended the transitions using
    // future_spec_, shift back to a supported year using the 400-year
    // cycle of calendaric equivalence and then compensate accordingly.
    if (extended_) {
      const int64_t diff = unix_time - transitions_[timecnt - 1].unix_time;
      const int64_t shift = diff / kSecPer400Years + 1;
      const auto d = seconds64(shift * kSecPer400Years);
      Breakdown bd = BreakTime(tp - d);
      bd.year += shift * 400;
      return bd;
    }
    const int type_index = transitions_[timecnt - 1].type_index;
    return LocalTime(unix_time, transition_types_[type_index]);
  }

  const int32_t hint = local_time_hint_.load(std::memory_order_relaxed);
  if (0 < hint && hint < timecnt) {
    if (unix_time < transitions_[hint].unix_time) {
      if (!(unix_time < transitions_[hint - 1].unix_time)) {
        const int type_index = transitions_[hint - 1].type_index;
        return LocalTime(unix_time, transition_types_[type_index]);
      }
    }
  }

  const Transition target = {unix_time, 0, {0}, {0}};
  const Transition* begin = &transitions_[0];
  const Transition* tr = std::upper_bound(begin, begin + timecnt, target,
                                          Transition::ByUnixTime());
  local_time_hint_.store(tr - begin, std::memory_order_relaxed);
  const int type_index = (--tr)->type_index;
  return LocalTime(unix_time, transition_types_[type_index]);
}

TimeInfo TimeZoneInfo::MakeTimeInfo(int64_t year, int mon, int day,
                                    int hour, int min, int sec) const {
  Transition target;
  DateTime& dt(target.date_time);
  const bool normalized = dt.Normalize(year, mon, day, hour, min, sec);

  const int32_t timecnt = transitions_.size();
  if (timecnt == 0) {
    // Use the default offset.
    int32_t offset = transition_types_[default_transition_type_].utc_offset;
    int64_t unix_time = (dt - DateTime{0}) - offset;
    return MakeUnique(unix_time, normalized);
  }

  // Find the first transition after our target date/time.
  const Transition* tr = nullptr;
  const Transition* begin = &transitions_[0];
  const Transition* end = begin + timecnt;
  if (dt < begin->date_time) {
    tr = begin;
  } else if (!(dt < transitions_[timecnt - 1].date_time)) {
    tr = end;
  } else {
    const int32_t hint = time_local_hint_.load(std::memory_order_relaxed);
    if (0 < hint && hint < timecnt) {
      if (dt < transitions_[hint].date_time) {
        if (!(dt < transitions_[hint - 1].date_time)) {
          tr = begin + hint;
        }
      }
    }
    if (tr == nullptr) {
      tr = std::upper_bound(begin, end, target, Transition::ByDateTime());
      time_local_hint_.store(tr - begin, std::memory_order_relaxed);
    }
  }

  if (tr == begin) {
    if (!(tr->prev_date_time < dt)) {
      // Before first transition, so use the default offset.
      int offset = transition_types_[default_transition_type_].utc_offset;
      int64_t unix_time = (dt - DateTime{0}) - offset;
      return MakeUnique(unix_time, normalized);
    }
    // tr->prev_date_time < dt < tr->date_time
    return MakeSkipped(*tr, dt, normalized);
  }

  if (tr == end) {
    if ((--tr)->prev_date_time < dt) {
      // After the last transition. If we extended the transitions using
      // future_spec_, shift back to a supported year using the 400-year
      // cycle of calendaric equivalence and then compensate accordingly.
      if (extended_ && year > last_year_) {
        const int64_t shift = (year - last_year_) / 400 + 1;
        return TimeLocal(year - shift * 400, mon, day, hour, min, sec,
                         shift * kSecPer400Years);
      }
      int64_t unix_time = tr->unix_time + (dt - tr->date_time);
      return MakeUnique(unix_time, normalized);
    }
    // tr->date_time <= dt <= tr->prev_date_time
    return MakeRepeated(*tr, dt, normalized);
  }

  if (tr->prev_date_time < dt) {
    // tr->prev_date_time < dt < tr->date_time
    return MakeSkipped(*tr, dt, normalized);
  }

  if (!((--tr)->prev_date_time < dt)) {
    // tr->date_time <= dt <= tr->prev_date_time
    return MakeRepeated(*tr, dt, normalized);
  }

  // In between transitions.
  int64_t unix_time = tr->unix_time + (dt - tr->date_time);
  return MakeUnique(unix_time, normalized);
}

}  // namespace cctz
