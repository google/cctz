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

#ifndef CCTZ_CIVIL_STRING_DETAIL_H_
#define CCTZ_CIVIL_STRING_DETAIL_H_

#include <cctype>
#include <cstring>
#include <string>

namespace cctz {
namespace detail {

struct char_range {
  char_range(const char* s)
      : begin(s), end(s + strlen(s)) {}
  char_range(const char* begin, const char* end)
      : begin(begin), end(end) {}
  char_range(const std::string& s)
      : begin(s.data()), end(s.data() + s.size()) {}
  char_range(const char_range&) = default;
  char_range& operator=(const char_range&) = default;

  friend bool operator==(char_range lhs, char_range rhs) {
    if (lhs.begin == rhs.begin && lhs.end == rhs.end) return true;
    if (lhs.size() != rhs.size()) return false;
    return memcmp(lhs.begin, rhs.begin, lhs.size()) == 0;
  }

  friend bool operator!=(char_range lhs, char_range rhs) {
    return !(lhs == rhs);
  }

  bool starts_with(char_range s) const {
    if (size() < s.size()) return false;
    return memcmp(begin, s.begin, s.size()) == 0;
  }

  bool consume_prefix(char_range s) {
    if (starts_with(s)) {
      begin += s.size();
      return true;
    }
    return false;
  }

  bool starts_with(char c) const {
    if (begin == end) return false;
    return *begin == c;
  }

  bool consume_prefix(char c) {
    if (starts_with(c)) {
      ++begin;
      return true;
    }
    return false;
  }

  bool consume_leading_spaces() {
    bool consumed = false;
    while (begin != end) {
      if (!std::isspace(*begin)) return consumed;
      ++begin;
      consumed = true;
    }
    return consumed;
  }

  size_t size() const { return end - begin; }

  const char* begin;
  const char* end;
};

}  // namespace detail
}  // namespace cctz

#endif  // CCTZ_CIVIL_STRING_DETAIL_H_
