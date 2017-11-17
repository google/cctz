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

#include <chrono>
#include <iostream>
#include <string>

#include "cctz/civil_time.h"
#include "cctz/time_zone.h"

template <typename D>
cctz::time_point<cctz::sys_seconds> FloorDay(cctz::time_point<D> tp,
                                             cctz::time_zone tz) {
  return cctz::convert(cctz::civil_day(cctz::convert(tp, tz)), tz);
}

int main() {
  cctz::time_zone lax;
  load_time_zone("America/Los_Angeles", &lax);
  const auto now = std::chrono::system_clock::now();
  const auto day = FloorDay(now, lax);
  std::cout << cctz::format("Now: %Y-%m-%d %H:%M:%S %z\n", now, lax);
  std::cout << cctz::format("Day: %Y-%m-%d %H:%M:%S %z\n", day, lax);
}
