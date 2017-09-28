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
  cctz::time_zone syd;
  if (!cctz::load_time_zone("Australia/Sydney", &syd)) return -1;

  // Neil Armstrong first walks on the moon
  const auto tp1 =
      cctz::convert(cctz::civil_second(1969, 7, 21, 12, 56, 0), syd);

  const std::string s = cctz::format("%Y-%m-%d %H:%M:%S %z", tp1, syd);
  std::cout << s << "\n";

  cctz::time_zone nyc;
  cctz::load_time_zone("America/New_York", &nyc);

  const auto tp2 =
      cctz::convert(cctz::civil_second(1969, 7, 20, 22, 56, 0), nyc);
  return tp2 == tp1 ? 0 : 1;
}
