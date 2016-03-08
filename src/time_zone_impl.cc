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

#include "time_zone_impl.h"

#include <map>
#include <mutex>

namespace cctz {

namespace {

// time_zone::Impls are linked into a map to support fast lookup by name.
typedef std::map<std::string, const time_zone::Impl*> TimeZoneImplByName;
TimeZoneImplByName* time_zone_map = nullptr;

// Mutual exclusion for time_zone_map.
std::mutex time_zone_mutex;

// The utc_time_zone(). Also used for time zones that fail to load.
const time_zone::Impl* utc_zone = nullptr;

// utc_zone should only be referenced in a thread that has just done
// a LoadUTCTimeZone().
std::once_flag load_utc_once;
void LoadUTCTimeZone() {
  std::call_once(load_utc_once, []() { utc_time_zone(); });
}

}  // namespace

bool time_zone::Impl::LoadTimeZone(const std::string& name, time_zone* tz) {
  const bool is_utc = (name.compare("UTC") == 0);

  // First check, under a shared lock, whether the time zone has already
  // been loaded. This is the common path. TODO: Move to shared_mutex.
  {
    std::lock_guard<std::mutex> lock(time_zone_mutex);
    if (time_zone_map != nullptr) {
      TimeZoneImplByName::const_iterator itr = time_zone_map->find(name);
      if (itr != time_zone_map->end()) {
        *tz = time_zone(itr->second);
        return is_utc || itr->second != utc_zone;
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
  const time_zone::Impl*& impl = (*time_zone_map)[name];
  bool fallback_utc = false;
  if (impl == nullptr) {
    // The first thread in loads the new time zone.
    time_zone::Impl* new_impl = new time_zone::Impl(name);
    new_impl->zone_ = TimeZoneIf::Load(new_impl->name_);
    if (new_impl->zone_ == nullptr) {
      delete new_impl;  // free the nascent time_zone::Impl
      impl = utc_zone;  // and fallback to UTC
      fallback_utc = true;
    } else {
      if (is_utc) {
        // Happens before any reference to utc_zone.
        utc_zone = new_impl;
      }
      impl = new_impl;  // install new time zone
    }
  }
  *tz = time_zone(impl);
  return !fallback_utc;
}

const time_zone::Impl& time_zone::Impl::get(const time_zone& tz) {
  if (tz.impl_ == nullptr) {
    // Dereferencing an implicit-UTC time_zone is expected to be
    // rare, so we don't mind paying a small synchronization cost.
    LoadUTCTimeZone();
    return *utc_zone;
  }
  return *tz.impl_;
}

time_zone::Impl::Impl(const std::string& name) : name_(name) {}

time_zone::absolute_lookup time_zone::Impl::BreakTime(
    const time_point<sys_seconds>& tp) const {
  time_zone::absolute_lookup res;
  Breakdown bd = zone_->BreakTime(tp);
  // TODO: Eliminate extra normalization.
  res.cs =
      civil_second(bd.year, bd.month, bd.day, bd.hour, bd.minute, bd.second);
  res.offset = bd.offset;
  res.is_dst = bd.is_dst;
  res.abbr = bd.abbr;
  return res;
}

time_zone::civil_lookup time_zone::Impl::MakeTimeInfo(civil_second cs) const {
  time_zone::civil_lookup res;
  // TODO: Eliminate extra normalization.
  TimeInfo t = zone_->MakeTimeInfo(cs.year(), cs.month(), cs.day(),
                                   cs.hour(), cs.minute(), cs.second());
  res.kind = t.kind;
  res.pre = t.pre;
  res.trans = t.trans;
  res.post = t.post;
  return res;
}

}  // namespace cctz
