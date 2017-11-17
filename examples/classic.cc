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

#include <ctime>
#include <iostream>
#include <string>

std::string format(const std::string& fmt, const std::tm& tm) {
  char buf[100];
  std::strftime(buf, sizeof(buf), fmt.c_str(), &tm);
  return buf;
}

int main() {
  const std::time_t now = std::time(nullptr);

  std::tm tm_utc;
#if defined(_WIN32) || defined(_WIN64)
  gmtime_s(&tm_utc, &now);
#else
  gmtime_r(&now, &tm_utc);
#endif
  std::cout << format("UTC: %Y-%m-%d %H:%M:%S\n", tm_utc);

  std::tm tm_local;
#if defined(_WIN32) || defined(_WIN64)
  localtime_s(&tm_local, &now);
#else
  localtime_r(&now, &tm_local);
#endif
  std::cout << format("Local: %Y-%m-%d %H:%M:%S\n", tm_local);
}
