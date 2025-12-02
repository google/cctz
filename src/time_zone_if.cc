// Copyright 2016 Google Inc. All Rights Reserved.
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

#include "time_zone_if.h"
#include "time_zone_info.h"
#include "time_zone_libc.h"

#if defined(_WIN32) && defined(CCTZ_USE_WIN_REGISTRY_FALLBACK)
#include "time_zone_win.h"
#include "time_zone_win_loader.h"
#endif  // defined(_WIN32) && defined(CCTZ_USE_WIN_REGISTRY_FALLBACK)

namespace cctz {

std::unique_ptr<TimeZoneIf> TimeZoneIf::UTC() {
  return TimeZoneInfo::UTC();
}

std::unique_ptr<TimeZoneIf> TimeZoneIf::Make(const std::string& name) {
  // Support "libc:localtime" and "libc:*" to access the legacy
  // localtime and UTC support respectively from the C library.
  // NOTE: The "libc:*" zones are internal, test-only interfaces, and
  // are subject to change/removal without notice. Do not use them.
  if (name.compare(0, 5, "libc:") == 0) {
    return TimeZoneLibC::Make(name.substr(5));
  }

  // Attempt to use the "zoneinfo" implementation.
  std::unique_ptr<TimeZoneIf> zone_info = TimeZoneInfo::Make(name);

  #if defined(_WIN32) && defined(CCTZ_USE_WIN_REGISTRY_FALLBACK)
  if (!zone_info) {
    // Attempt to fall back to Win32 Registry Implementation.
    zone_info = MakeTimeZoneFromWinRegistry(LoadWinTimeZoneRegistry(name));
  }
  #endif  // defined(_WIN32) && defined(CCTZ_USE_WIN_REGISTRY_FALLBACK)

  return zone_info;
}

// Defined out-of-line to avoid emitting a weak vtable in all TUs.
TimeZoneIf::~TimeZoneIf() {}

}  // namespace cctz
