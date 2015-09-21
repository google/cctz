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

#include <iostream>
#include <string>

#include "src/cctz.h"

cctz::time_point FloorDay(cctz::time_point tp, cctz::TimeZone tz) {
  const cctz::Breakdown bd = cctz::BreakTime(tp, tz);
  const cctz::TimeInfo ti =
      cctz::MakeTimeInfo(bd.year, bd.month, bd.day, 0, 0, 0, tz);
  if (ti.kind == cctz::TimeInfo::Kind::SKIPPED) return ti.trans;
  return ti.pre;
}

int main() {
  cctz::TimeZone lax;
  LoadTimeZone("America/Los_Angeles", &lax);
  const auto now = std::chrono::system_clock::now();
  const auto day = FloorDay(now, lax);
  std::cout << cctz::Format("Now: %F %T %z\n", now, lax);
  std::cout << cctz::Format("Day: %F %T %z\n", day, lax);
}
