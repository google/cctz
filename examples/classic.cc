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

#include <cstddef>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <string>

std::string format(const std::string& fmt, const std::tm& tm) {
  char buf[100];
  std::strftime(buf, sizeof(buf), fmt.c_str(), &tm);
  return buf;
}

int main() {
  const std::time_t now = std::time(nullptr);

  std::tm tm_utc;
  gmtime_r(&now, &tm_utc);
  std::cout << format("UTC: %F %T\n", tm_utc);

  std::tm tm_local;
  localtime_r(&now, &tm_local);
  std::cout << format("Local: %F %T\n", tm_local);
}
