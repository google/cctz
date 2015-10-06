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

int main() {
  cctz::TimeZone lax;
  LoadTimeZone("America/Los_Angeles", &lax);

  const auto now = std::chrono::system_clock::now();
  const cctz::Breakdown bd = cctz::BreakTime(now, lax);

  // First day of month, 6 months from now.
  const auto then = cctz::MakeTime(bd.year, bd.month + 6, 1, 0, 0, 0, lax);

  std::cout << cctz::Format("Now: %F %T %z\n", now, lax);
  std::cout << cctz::Format("6mo: %F %T %z\n", then, lax);
}
