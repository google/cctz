// Copyright 2025 Google Inc. All Rights Reserved.
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

#ifndef CCTZ_TIME_ZONE_WIN_LOADER_H_
#define CCTZ_TIME_ZONE_WIN_LOADER_H_

#if defined(_WIN32)
#include <string>

#include "time_zone_win.h"

namespace cctz {

WinTimeZoneRegistryInfo LoadWinTimeZoneRegistry(const std::string& name);

}  // namespace cctz

#endif  // defined(_WIN32)
#endif  // CCTZ_TIME_ZONE_WIN_LOADER_H_
