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

#ifndef CCTZ_IMPL_H_
#define CCTZ_IMPL_H_

#include <memory>
#include <string>

#include "src/cctz.h"
#include "src/cctz_info.h"

namespace cctz {

// TimeZone::Impl is the internal object referenced by a cctz::TimeZone.
class TimeZone::Impl {
 public:
  // Load a named time zone. Returns false if the name is invalid, or if
  // some other kind of error occurs. Note that loading "UTC" never fails.
  static bool LoadTimeZone(const std::string& name, TimeZone* tz);

  // Dereferences the TimeZone to obtain its Impl.
  static const TimeZone::Impl& get(const TimeZone& tz);

  // Breaks a time_point down to civil-time components in this time zone.
  Breakdown BreakTime(const time_point<seconds64>& tp) const;

  // Converts the civil-time components in this time zone into a time_point.
  // That is, the opposite of BreakTime(). The requested civil time may be
  // ambiguous or illegal due to a change of UTC offset.
  TimeInfo MakeTimeInfo(int64_t year, int mon, int day,
                        int hour, int min, int sec) const;

 private:
  explicit Impl(const std::string& name);

  const std::string name_;
  std::unique_ptr<TimeZoneIf> zone_;
};

}  // namespace cctz

#endif  // CCTZ_IMPL_H_
