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

#include "time_zone_fixed.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>

namespace cctz {

namespace {

// The prefix used for the internal names of fixed-offset zones.
const char kFixedOffsetPrefix[] = "Fixed/";

int Parse02d(const char* p) {
  static const char kDigits[] = "0123456789";
  if (const char* ap = std::strchr(kDigits, *p)) {
    int v = static_cast<int>(ap - kDigits);
    if (const char* bp = std::strchr(kDigits, *++p)) {
      return (v * 10) + static_cast<int>(bp - kDigits);
    }
  }
  return -1;
}

}  // namespace

bool FixedOffsetFromName(const std::string& name, sys_seconds* offset) {
  if (name.compare(0, std::string::npos, "UTC", 3) == 0) {
    *offset = sys_seconds::zero();
    return true;
  }

  const std::size_t prefix_len = sizeof(kFixedOffsetPrefix) - 1;
  const char* const ep = kFixedOffsetPrefix + prefix_len;
  if (name.size() != prefix_len + 12)  // "<prefix>UTC+99:99:99"
    return false;
  if (!std::equal(kFixedOffsetPrefix, ep, name.begin()))
    return false;
  const char* np = name.data() + prefix_len;
  if (*np++ != 'U' || *np++ != 'T' || *np++ != 'C')
    return false;
  if (np[0] != '+' && np[0] != '-')
    return false;
  if (np[3] != ':' || np[6] != ':')  // see note below about large offsets
    return false;

  int hours = Parse02d(np + 1);
  if (hours == -1) return false;
  int mins = Parse02d(np + 4);
  if (mins == -1) return false;
  int secs = Parse02d(np + 7);
  if (secs == -1) return false;

  secs += ((hours * 60) + mins) * 60;
  if (secs > 24 * 60 * 60) return false;  // outside supported offset range
  *offset = sys_seconds(secs * (np[0] == '-' ? -1 : 1));  // "-" means west
  return true;
}

std::string FixedOffsetToName(const sys_seconds& offset) {
  if (offset == sys_seconds::zero()) return "UTC";
  if (offset < std::chrono::hours(-24) || offset > std::chrono::hours(24)) {
    // We don't support fixed-offset zones more than 24 hours
    // away from UTC to avoid complications in rendering such
    // offsets and to (somewhat) limit the total number of zones.
    return "UTC";
  }
  int seconds = static_cast<int>(offset.count());
  const char sign = (seconds < 0 ? '-' : '+');
  int minutes = seconds / 60;
  seconds %= 60;
  if (sign == '-') {
    if (seconds > 0) {
      seconds -= 60;
      minutes += 1;
    }
    seconds = -seconds;
    minutes = -minutes;
  }
  int hours = minutes / 60;
  minutes %= 60;
  char buf[sizeof(kFixedOffsetPrefix) + sizeof("UTC-24:00:00")];
  snprintf(buf, sizeof(buf), "%sUTC%c%02d:%02d:%02d",
           kFixedOffsetPrefix, sign, hours, minutes, seconds);
  return buf;
}

std::string FixedOffsetToAbbr(const sys_seconds& offset) {
  std::string abbr = FixedOffsetToName(offset);
  const std::size_t prefix_len = sizeof(kFixedOffsetPrefix) - 1;
  const char* const ep = kFixedOffsetPrefix + prefix_len;
  if (abbr.size() >= prefix_len) {
    if (std::equal(kFixedOffsetPrefix, ep, abbr.begin())) {
      abbr.erase(0, prefix_len);
      if (abbr.size() == 12) {                     // UTC+99:99:99
        abbr.erase(9, 1);                          // UTC+99:9999
        abbr.erase(6, 1);                          // UTC+999999
        if (abbr[8] == '0' && abbr[9] == '0') {    // UTC+999900
          abbr.erase(8, 2);                        // UTC+9999
          if (abbr[6] == '0' && abbr[7] == '0') {  // UTC+9900
            abbr.erase(6, 2);                      // UTC+99
            if (abbr[4] == '0') {                  // UTC+09
              abbr.erase(4, 1);                    // UTC+9
            }
          }
        }
      }
    }
  }
  return abbr;
}

}  // namespace cctz
