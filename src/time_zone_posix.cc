// Copyright 2016 Google Inc. All Rights Reserved.
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

#include "time_zone_posix.h"

#include <cstddef>
#include <cstring>
#include <limits>
#include <string>

namespace cctz {

namespace {

const char* ParseInt(detail::char_range s, int min, int max, int* vp) {
  int value = 0;
  const int kMaxInt = std::numeric_limits<int>::max();
  const char* p = s.begin;
  while (p != s.end) {
    int d = *p - '0';
    if (d < 0 || 10 <= d) break;
    if (value > kMaxInt / 10) return nullptr;
    value *= 10;
    if (value > kMaxInt - d) return nullptr;
    value += d;
    ++p;
  }
  if (p == s.begin || value < min || max < value) return nullptr;
  *vp = value;
  return p;
}

// abbr = <.*?> | [^-+,\d]{3,}
const char* ParseAbbr(detail::char_range s, std::string* abbr) {
  const char* p = s.begin;
  if (*p == '<') {  // special zoneinfo <...> form
    while (*++p != '>') {
      if (p == s.end) return nullptr;
    }
    size_t size = p - s.begin;
    abbr->assign(s.begin + 1, size - 1);
    return p + 1;
  }
  while (p != s.end) {
    if (strchr("-+,0123456789", *p)) break;
    ++p;
  }
  size_t size = p - s.begin;
  if (size < 3) return nullptr;
  abbr->assign(s.begin, size);
  return p;
}

// offset = [+|-]hh[:mm[:ss]] (aggregated into single seconds value)
const char* ParseOffset(detail::char_range s, int min_hour, int max_hour, int sign,
                        std::int_fast32_t* offset) {
  if (s.consume_prefix('-')) {
    sign = -sign;
  } else {
    s.consume_prefix('+');
  }

  int hours = 0;
  int minutes = 0;
  int seconds = 0;

  s.begin = ParseInt(s, min_hour, max_hour, &hours);
  if (s.begin == nullptr) return nullptr;
  if (s.consume_prefix(':')) {
    s.begin = ParseInt(s, 0, 59, &minutes);
    if (s.begin == nullptr) return nullptr;
    if (s.consume_prefix(':')) {
      s.begin = ParseInt(s, 0, 59, &seconds);
      if (s.begin == nullptr) return nullptr;
    }
  }
  *offset = sign * ((((hours * 60) + minutes) * 60) + seconds);
  return s.begin;
}

// datetime = ( Jn | n | Mm.w.d ) [ / offset ]
const char* ParseDateTime(detail::char_range s, PosixTransition* res) {
  if (s.consume_prefix(',')) {
    if (s.consume_prefix('M')) {
      int month = 0;
      s.begin = ParseInt(s, 1, 12, &month);
      if (s.begin == nullptr) return nullptr;
      if (s.consume_prefix('.')) {
        int week = 0;
        s.begin = ParseInt(s, 1, 5, &week);
        if (s.begin == nullptr) return nullptr;
        if (s.consume_prefix('.')) {
          int weekday = 0;
          s.begin =  ParseInt(s, 0, 6, &weekday);
          if (s.begin == nullptr) return nullptr;
          res->date.fmt = PosixTransition::M;
          res->date.m.month = static_cast<std::int_fast8_t>(month);
          res->date.m.week = static_cast<std::int_fast8_t>(week);
          res->date.m.weekday = static_cast<std::int_fast8_t>(weekday);
        }
      }
    } else if (s.consume_prefix('J')) {
      int day = 0;
      s.begin = ParseInt(s, 1, 365, &day);
      if (s.begin == nullptr) return nullptr;
      res->date.fmt = PosixTransition::J;
      res->date.j.day = static_cast<std::int_fast16_t>(day);
    } else {
      int day = 0;
      s.begin =  ParseInt(s, 0, 365, &day);
      if (s.begin == nullptr) return nullptr;
      res->date.fmt = PosixTransition::N;
      res->date.n.day = static_cast<std::int_fast16_t>(day);
    }
  }
  res->time.offset = 2 * 60 * 60;  // default offset is 02:00:00
  if (s.consume_prefix('/')) {
    s.begin = ParseOffset(s, -167, 167, 1, &res->time.offset);
    if (s.begin == nullptr) return nullptr;
  }
  return s.begin;
}

}  // namespace

// spec = std offset [ dst [ offset ] , datetime , datetime ]
bool ParsePosixSpec(detail::char_range spec, PosixTimeZone* res) {
  if (spec.starts_with(':')) return false;

  spec.begin = ParseAbbr(spec, &res->std_abbr);
  if (spec.begin == nullptr) return false;

  spec.begin = ParseOffset(spec, 0, 24, -1, &res->std_offset);
  if (spec.begin == nullptr) return false;
  if (spec.begin == spec.end) return true;

  spec.begin = ParseAbbr(spec, &res->dst_abbr);
  if (spec.begin == nullptr) return false;
  res->dst_offset = res->std_offset + (60 * 60);  // default
  if (!spec.starts_with(',')) {
    spec.begin = ParseOffset(spec.begin, 0, 24, -1, &res->dst_offset);
    if (spec.begin == nullptr) return false;
  }

  spec.begin = ParseDateTime(spec, &res->dst_start);
  if (spec.begin == nullptr) return false;
  spec.begin = ParseDateTime(spec, &res->dst_end);
  return spec.begin == spec.end;
}

}  // namespace cctz
