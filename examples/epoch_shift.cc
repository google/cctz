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

int GetOffset(std::time_t, const std::string& zone) {
  if (zone == "America/New_York") return -4 * 60 * 60;
  return 0;
}

int main() {
  const std::time_t now = std::time(nullptr);

  // Shift epoch: UTC to "local time_t"
  int off = GetOffset(now, "America/New_York");
  const std::time_t now_nyc = now + off;
  std::tm tm_nyc;
#if defined(_WIN32) || defined(_WIN64)
  gmtime_s(&tm_nyc, &now_nyc);
#else
  gmtime_r(&now_nyc, &tm_nyc);
#endif
  std::cout << format("NYC: %Y-%m-%d %H:%M:%S\n", tm_nyc);

  // Shift back: "local time_t" to UTC
  off = GetOffset(now_nyc, "America/New_York");
  const std::time_t now_utc = now_nyc - off;
  return now_utc == now ? 0 : 1;
}
