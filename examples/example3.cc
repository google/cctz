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

int main() {
  cctz::time_zone lax;
  load_time_zone("America/Los_Angeles", &lax);

  const auto now = std::chrono::system_clock::now();
  const cctz::civil_second cs = cctz::convert(now, lax);

  // First day of month, 6 months from now.
  const auto then = cctz::convert(cctz::civil_month(cs) + 6, lax);

  std::cout << cctz::format("Now: %Y-%m-%d %H:%M:%S %z\n", now, lax);
  std::cout << cctz::format("6mo: %Y-%m-%d %H:%M:%S %z\n", then, lax);
}
