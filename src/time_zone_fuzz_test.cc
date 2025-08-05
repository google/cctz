// Copyright 2025 Google Inc. All Rights Reserved.
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

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

#include "cctz/civil_time.h"
#include "cctz/time_zone.h"
#include "fuzztest/fuzztest.h"
#include "gtest/gtest.h"
#include "test_time_zone_names.h"
#include "time_zone_impl.h"

namespace cctz {

namespace {

std::vector<std::string> TimeZoneSeeds() {
  std::vector<std::string> time_zone_seeds;
  for (const char* const* tzptr = kTimeZoneNames; *tzptr != nullptr; ++tzptr) {
    time_zone_seeds.push_back(*tzptr);
  }
  return time_zone_seeds;
}

void Fuzz(const std::string& time_zone_name, const std::int64_t year,
          const std::int64_t month, const std::int64_t day,
          const std::int64_t hour, const std::int64_t minute,
          const std::int64_t second, const std::string& format_string,
          const std::string& cctz_parse_input) {
  cctz::time_zone time_zone;
  if (!cctz::load_time_zone(time_zone_name, &time_zone)) {
    return;
  }
  const cctz::civil_second cs(year, month, day, hour, minute, second);
  auto time_point = cctz::convert(cs, time_zone);
  cctz::format(format_string, time_point, time_zone);
  cctz::time_point<cctz::seconds> parsed_time_point;
  cctz::parse(format_string, cctz_parse_input, time_zone, &parsed_time_point);
  auto repeated_time_point = cctz::convert(cs, time_zone);
  EXPECT_EQ(time_point, repeated_time_point);
}

FUZZ_TEST(TimeZoneFuzzTest, Fuzz)
    .WithDomains(fuzztest::Arbitrary<std::string>().WithSeeds(TimeZoneSeeds()),
                 // We limit the search-space for years to avoid expected
                 // undefined behavior that may occur when normalizing a civil
                 // second outside the 64-bit representable year range.
                 fuzztest::InRange(std::numeric_limits<std::int64_t>::min() / 2,
                                   std::numeric_limits<std::int64_t>::max() /
                                       2),             // year
                 fuzztest::Arbitrary<std::int64_t>(),  // month
                 fuzztest::Arbitrary<std::int64_t>(),  // day
                 fuzztest::Arbitrary<std::int64_t>(),  // hour
                 fuzztest::Arbitrary<std::int64_t>(),  // minute
                 fuzztest::Arbitrary<std::int64_t>(),  // second
                 fuzztest::Arbitrary<std::string>(),   // format_string
                 fuzztest::Arbitrary<std::string>());  // cctz_parse_input

}  // namespace

}  // namespace cctz
