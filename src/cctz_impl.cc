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

#include "src/cctz_impl.h"

#include <map>
#include <mutex>

namespace cctz {

namespace {

// TimeZone::Impls are linked into a map to support fast lookup by name.
typedef std::map<std::string, const TimeZone::Impl*> TimeZoneImplByName;
TimeZoneImplByName* time_zone_map = nullptr;

// Mutual exclusion for time_zone_map.
std::mutex time_zone_mutex;

// The UTCTimeZone(). Also used for time zones that fail to load.
const TimeZone::Impl* utc_time_zone = nullptr;

// utc_time_zone should only be referenced in a thread that has just done
// a LoadUTCTimeZone().
std::once_flag load_utc_once;
void LoadUTCTimeZone() {
  std::call_once(load_utc_once, []() { UTCTimeZone(); });
}

}  // namespace

bool TimeZone::Impl::LoadTimeZone(const std::string& name, TimeZone* tz) {
  const bool is_utc = (name.compare("UTC") == 0);

  // First check, under a shared lock, whether the time zone has already
  // been loaded. This is the common path. TODO: Move to shared_mutex.
  {
    std::lock_guard<std::mutex> lock(time_zone_mutex);
    if (time_zone_map != nullptr) {
      TimeZoneImplByName::const_iterator itr = time_zone_map->find(name);
      if (itr != time_zone_map->end()) {
        *tz = TimeZone(itr->second);
        return is_utc || itr->second != utc_time_zone;
      }
    }
  }

  if (!is_utc) {
    // Ensure that UTC is loaded before any other time zones.
    LoadUTCTimeZone();
  }

  // Now check again, under an exclusive lock.
  std::lock_guard<std::mutex> lock(time_zone_mutex);
  if (time_zone_map == nullptr) time_zone_map = new TimeZoneImplByName;
  const TimeZone::Impl*& impl = (*time_zone_map)[name];
  bool fallback_utc = false;
  if (impl == nullptr) {
    // The first thread in loads the new time zone.
    TimeZone::Impl* new_impl = new TimeZone::Impl(name);
    new_impl->zone_ = TimeZoneIf::Load(new_impl->name_);
    if (new_impl->zone_ == nullptr) {
      delete new_impl;       // free the nascent TimeZone::Impl
      impl = utc_time_zone;  // and fallback to UTC
      fallback_utc = true;
    } else {
      if (is_utc) {
        // Happens before any reference to utc_time_zone.
        utc_time_zone = new_impl;
      }
      impl = new_impl;  // install new time zone
    }
  }
  *tz = TimeZone(impl);
  return !fallback_utc;
}

const TimeZone::Impl& TimeZone::Impl::get(const TimeZone& tz) {
  if (tz.impl_ == nullptr) {
    // Dereferencing an implicit-UTC TimeZone is expected to be
    // rare, so we don't mind paying a small synchronization cost.
    LoadUTCTimeZone();
    return *utc_time_zone;
  }
  return *tz.impl_;
}

TimeZone::Impl::Impl(const std::string& name) : name_(name) {}

Breakdown TimeZone::Impl::BreakTime(const time_point<seconds64>& tp) const {
  return zone_->BreakTime(tp);
}

TimeInfo TimeZone::Impl::MakeTimeInfo(int64_t year, int mon, int day,
                                      int hour, int min, int sec) const {
  return zone_->MakeTimeInfo(year, mon, day, hour, min, sec);
}

}  // namespace cctz
