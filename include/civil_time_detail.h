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

#include <iomanip>
#include <limits>
#include <ostream>
#include <sstream>
#include <type_traits>

// Disable constexpr support unless we are using clang in C++14 mode.
#if __clang__ && __cpp_constexpr >= 201304
#define CONSTEXPR_D constexpr  // data
#define CONSTEXPR_F constexpr  // function
#define CONSTEXPR_M constexpr  // member
#define CONSTEXPR_T constexpr  // template
#else
#define CONSTEXPR_D const
#define CONSTEXPR_F inline
#define CONSTEXPR_M
#define CONSTEXPR_T
#endif

namespace cctz {
namespace detail {

// Normalized civil-time fields: Y-M-D HH:MM:SS.
struct fields {
  int y;
  int m;
  int d;
  int hh;
  int mm;
  int ss;
};

struct second_tag {};
struct minute_tag : second_tag {};
struct hour_tag : minute_tag {};
struct day_tag : hour_tag {};
struct month_tag : day_tag {};
struct year_tag : month_tag {};

////////////////////////////////////////////////////////////////////////

// Field normalization (without avoidable overflow).

namespace impl {

CONSTEXPR_F bool is_leap_year(int y) noexcept {
  return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}
CONSTEXPR_F int year_index(int y, int m) noexcept {
  return (((y + (m > 2)) % 400) + 400) % 400;
}
CONSTEXPR_F int days_per_century(int y, int m) noexcept {
  const int yi = year_index(y, m);
  return 36524 + (yi == 0 || yi > 300);
}
CONSTEXPR_F int days_per_4years(int y, int m) noexcept {
  const int yi = year_index(y, m);
  return 1460 + (yi == 0 || yi > 300 || (yi - 1) % 100 < 96);
}
CONSTEXPR_F int days_per_year(int y, int m) noexcept {
  return is_leap_year(y + (m > 2)) ? 366 : 365;
}
CONSTEXPR_F int days_per_month(int y, int m) noexcept {
  CONSTEXPR_D signed char k_days_per_month[12] = {
      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31  // non leap year
  };
  return k_days_per_month[m - 1] + (m == 2 && is_leap_year(y));
}

CONSTEXPR_F fields n_day(int y, int m, int d, int cd, int hh, int mm,
                         int ss) noexcept {
  y += (cd / 146097) * 400;
  cd %= 146097;
  if (cd < 0) {
    y -= 400;
    cd += 146097;
  }
  y += (d / 146097) * 400;
  d = d % 146097 + cd;
  if (d <= 0) {
    y -= 400;
    d += 146097;
  } else if (d > 146097) {
    y += 400;
    d -= 146097;
  }
  if (d > 365) {
    for (int n = days_per_century(y, m); d > n; n = days_per_century(y, m)) {
      d -= n;
      y += 100;
    }
    for (int n = days_per_4years(y, m); d > n; n = days_per_4years(y, m)) {
      d -= n;
      y += 4;
    }
    for (int n = days_per_year(y, m); d > n; n = days_per_year(y, m)) {
      d -= n;
      ++y;
    }
  }
  if (d > 28) {
    for (int n = days_per_month(y, m); d > n; n = days_per_month(y, m)) {
      d -= n;
      if (++m > 12) {
        ++y;
        m = 1;
      }
    }
  }
  return fields{y, m, d, hh, mm, ss};
}
CONSTEXPR_F fields n_mon(int y, int m, int d, int cd, int hh, int mm,
                         int ss) noexcept {
  y += m / 12;
  m %= 12;
  if (m <= 0) {
    y -= 1;
    m += 12;
  }
  return n_day(y, m, d, cd, hh, mm, ss);
}
CONSTEXPR_F fields n_hour(int y, int m, int d, int cd, int hh, int mm,
                          int ss) noexcept {
  cd += hh / 24;
  hh %= 24;
  if (hh < 0) {
    cd -= 1;
    hh += 24;
  }
  return n_mon(y, m, d, cd, hh, mm, ss);
}
CONSTEXPR_F fields n_min(int y, int m, int d, int hh, int ch, int mm,
                         int ss) noexcept {
  ch += mm / 60;
  mm %= 60;
  if (mm < 0) {
    ch -= 1;
    mm += 60;
  }
  return n_hour(y, m, d, hh / 24 + ch / 24, hh % 24 + ch % 24, mm, ss);
}
CONSTEXPR_F fields n_sec(int y, int m, int d, int hh, int mm, int ss) noexcept {
  int cm = ss / 60;
  ss %= 60;
  if (ss < 0) {
    cm -= 1;
    ss += 60;
  }
  return n_min(y, m, d, hh, mm / 60 + cm / 60, mm % 60 + cm % 60, ss);
}

}  // namespace impl

////////////////////////////////////////////////////////////////////////

// Increments the indicated (normalized) field by "n".
CONSTEXPR_F fields step(second_tag, fields f, int n) noexcept {
  return impl::n_sec(f.y, f.m, f.d, f.hh, f.mm + n / 60, f.ss + n % 60);
}
CONSTEXPR_F fields step(minute_tag, fields f, int n) noexcept {
  return impl::n_min(f.y, f.m, f.d, f.hh + n / 60, 0, f.mm + n % 60, f.ss);
}
CONSTEXPR_F fields step(hour_tag, fields f, int n) noexcept {
  return impl::n_hour(f.y, f.m, f.d + n / 24, 0, f.hh + n % 24, f.mm, f.ss);
}
CONSTEXPR_F fields step(day_tag, fields f, int n) noexcept {
  return impl::n_day(f.y, f.m, f.d, n, f.hh, f.mm, f.ss);
}
CONSTEXPR_F fields step(month_tag, fields f, int n) noexcept {
  return impl::n_mon(f.y + n / 12, f.m + n % 12, f.d, 0, f.hh, f.mm, f.ss);
}
CONSTEXPR_F fields step(year_tag, fields f, int n) noexcept {
  return fields{f.y + n, f.m, f.d, f.hh, f.mm, f.ss};
}

////////////////////////////////////////////////////////////////////////

// Aligns the (normalized) fields struct to the indicated field.
CONSTEXPR_F fields align(second_tag, fields f) noexcept {
  return f;
}
CONSTEXPR_F fields align(minute_tag, fields f) noexcept {
  return fields{f.y, f.m, f.d, f.hh, f.mm, 0};
}
CONSTEXPR_F fields align(hour_tag, fields f) noexcept {
  return fields{f.y, f.m, f.d, f.hh, 0, 0};
}
CONSTEXPR_F fields align(day_tag, fields f) noexcept {
  return fields{f.y, f.m, f.d, 0, 0, 0};
}
CONSTEXPR_F fields align(month_tag, fields f) noexcept {
  return fields{f.y, f.m, 1, 0, 0, 0};
}
CONSTEXPR_F fields align(year_tag, fields f) noexcept {
  return fields{f.y, 1, 1, 0, 0, 0};
}

////////////////////////////////////////////////////////////////////////

namespace impl {

// Map a (normalized) Y/M/D to the number of days before/after 1970-01-01.
// Will overflow outside of the range [-5877641-06-23 ... 5881580-07-11].
CONSTEXPR_F int ymd_ord(int y, int m, int d) noexcept {
  const int eyear = (m <= 2) ? y - 1 : y;
  const int era = (eyear >= 0 ? eyear : eyear - 399) / 400;
  const int yoe = eyear - era * 400;
  const int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;
}

}  // namespace impl

// Returns the difference between fields structs using the indicated unit.
CONSTEXPR_F int difference(year_tag, fields f1, fields f2) noexcept {
  return f1.y - f2.y;
}
CONSTEXPR_F int difference(month_tag, fields f1, fields f2) noexcept {
  return difference(year_tag{}, f1, f2) * 12 + (f1.m - f2.m);
}
CONSTEXPR_F int difference(day_tag, fields f1, fields f2) noexcept {
  return impl::ymd_ord(f1.y, f1.m, f1.d) - impl::ymd_ord(f2.y, f2.m, f2.d);
}
CONSTEXPR_F int difference(hour_tag, fields f1, fields f2) noexcept {
  return difference(day_tag{}, f1, f2) * 24 + (f1.hh - f2.hh);
}
CONSTEXPR_F int difference(minute_tag, fields f1, fields f2) noexcept {
  return difference(hour_tag{}, f1, f2) * 60 + (f1.mm - f2.mm);
}
CONSTEXPR_F int difference(second_tag, fields f1, fields f2) noexcept {
  return difference(minute_tag{}, f1, f2) * 60 + (f1.ss - f2.ss);
}

////////////////////////////////////////////////////////////////////////

template <typename T>
class civil_time {
 public:
  explicit CONSTEXPR_M civil_time(int y, int m = 1, int d = 1, int hh = 0,
                                  int mm = 0, int ss = 0) noexcept
      : civil_time(impl::n_sec(y, m, d, hh, mm, ss)) {}

  CONSTEXPR_M civil_time() noexcept : civil_time(1970) {}
  civil_time(const civil_time&) = default;
  civil_time& operator=(const civil_time&) = default;

  // Conversion between civil times of different alignment. Conversion to
  // a more precise alignment is allowed implicitly (e.g., day -> hour),
  // but conversion where information is discarded must be explicit
  // (e.g., second -> minute).
  template <typename U, typename S>
  using preserves_data =
      typename std::enable_if<std::is_base_of<U, S>::value>::type;
  template <typename U>
  CONSTEXPR_M civil_time(const civil_time<U>& ct,
                         preserves_data<T, U>* = nullptr) noexcept
      : civil_time(ct.f_) {}
  template <typename U>
  explicit CONSTEXPR_M civil_time(const civil_time<U>& ct,
                                  preserves_data<U, T>* = nullptr) noexcept
      : civil_time(ct.f_) {}

  // Factories for the maximum/minimum representable civil_time.
  static civil_time max() {
    return civil_time(std::numeric_limits<int>::max(), 12, 31, 23, 59, 59);
  }
  static civil_time min() {
    return civil_time(std::numeric_limits<int>::min(), 1, 1, 0, 0, 0);
  }

  // Field accessors.
  CONSTEXPR_M int year() const noexcept { return f_.y; }
  CONSTEXPR_M int month() const noexcept { return f_.m; }
  CONSTEXPR_M int day() const noexcept { return f_.d; }
  CONSTEXPR_M int hour() const noexcept { return f_.hh; }
  CONSTEXPR_M int minute() const noexcept { return f_.mm; }
  CONSTEXPR_M int second() const noexcept { return f_.ss; }

  // Assigning arithmetic.
  CONSTEXPR_M civil_time& operator+=(int n) noexcept {
    f_ = step(T{}, f_, n);
    return *this;
  }
  CONSTEXPR_M civil_time& operator-=(int n) noexcept {
    if (n != std::numeric_limits<int>::min()) {
      f_ = step(T{}, f_, -n);
    } else {
      f_ = step(T(), step(T{}, f_, -(n + 1)), 1);
    }
    return *this;
  }
  CONSTEXPR_M civil_time& operator++() noexcept {
    return *this += 1;
  }
  CONSTEXPR_M civil_time operator++(int) noexcept {
    const civil_time a = *this;
    ++*this;
    return a;
  }
  CONSTEXPR_M civil_time& operator--() noexcept {
    return *this -= 1;
  }
  CONSTEXPR_M civil_time operator--(int) noexcept {
    const civil_time a = *this;
    --*this;
    return a;
  }

  // Binary arithmetic operators.
  inline friend CONSTEXPR_M civil_time operator+(const civil_time& a,
                                                 int n) noexcept {
    return civil_time(step(T{}, a.f_, n));
  }
  inline friend CONSTEXPR_M civil_time operator+(int n,
                                                 const civil_time& a) noexcept {
    return civil_time(step(T{}, a.f_, n));
  }
  inline friend CONSTEXPR_M civil_time operator-(const civil_time& a,
                                                 int n) noexcept {
    return civil_time(step(T{}, a.f_, -n));
  }
  inline friend CONSTEXPR_M int operator-(const civil_time& lhs,
                                          const civil_time& rhs) noexcept {
    return difference(T{}, lhs.f_, rhs.f_);
  }

 private:
  // All instantiations of this template are allowed to call the following
  // private constructor and access the private fields member.
  template <typename U>
  friend class civil_time;

  // The designated constructor that all others eventually call.
  explicit CONSTEXPR_M civil_time(fields f) noexcept : f_(align(T{}, f)) {}

  fields f_;
};

using civil_year = civil_time<year_tag>;
using civil_month = civil_time<month_tag>;
using civil_day = civil_time<day_tag>;
using civil_hour = civil_time<hour_tag>;
using civil_minute = civil_time<minute_tag>;
using civil_second = civil_time<second_tag>;

////////////////////////////////////////////////////////////////////////

// Relational operators that work with differently aligned objects.
// Always compares all six fields.
template <typename T1, typename T2>
CONSTEXPR_T bool operator<(const civil_time<T1>& lhs,
                           const civil_time<T2>& rhs) noexcept {
  return (lhs.year() < rhs.year() ||
          (lhs.year() == rhs.year() &&
           (lhs.month() < rhs.month() ||
            (lhs.month() == rhs.month() &&
             (lhs.day() < rhs.day() ||
              (lhs.day() == rhs.day() &&
               (lhs.hour() < rhs.hour() ||
                (lhs.hour() == rhs.hour() &&
                 (lhs.minute() < rhs.minute() ||
                  (lhs.minute() == rhs.minute() &&
                   (lhs.second() < rhs.second())))))))))));
}
template <typename T1, typename T2>
CONSTEXPR_T bool operator<=(const civil_time<T1>& lhs,
                            const civil_time<T2>& rhs) noexcept {
  return !(rhs < lhs);
}
template <typename T1, typename T2>
CONSTEXPR_T bool operator>=(const civil_time<T1>& lhs,
                            const civil_time<T2>& rhs) noexcept {
  return !(lhs < rhs);
}
template <typename T1, typename T2>
CONSTEXPR_T bool operator>(const civil_time<T1>& lhs,
                           const civil_time<T2>& rhs) noexcept {
  return rhs < lhs;
}
template <typename T1, typename T2>
CONSTEXPR_T bool operator==(const civil_time<T1>& lhs,
                            const civil_time<T2>& rhs) noexcept {
  return lhs.year() == rhs.year() && lhs.month() == rhs.month() &&
         lhs.day() == rhs.day() && lhs.hour() == rhs.hour() &&
         lhs.minute() == rhs.minute() && lhs.second() == rhs.second();
}
template <typename T1, typename T2>
CONSTEXPR_T bool operator!=(const civil_time<T1>& lhs,
                            const civil_time<T2>& rhs) noexcept {
  return !(lhs == rhs);
}

////////////////////////////////////////////////////////////////////////

// Output stream operators output a format matching YYYY-MM-DDThh:mm:ss,
// while omitting fields inferior to the type's alignment. For example,
// civil_day is formatted only as YYYY-MM-DD.
inline std::ostream& operator<<(std::ostream& os, const civil_year& y) {
  std::stringstream ss;
  ss << y.year();  // No padding.
  return os << ss.str();
}
inline std::ostream& operator<<(std::ostream& os, const civil_month& m) {
  std::stringstream ss;
  ss << civil_year(m) << '-';
  ss << std::setfill('0') << std::setw(2) << m.month();
  return os << ss.str();
}
inline std::ostream& operator<<(std::ostream& os, const civil_day& d) {
  std::stringstream ss;
  ss << civil_month(d) << '-';
  ss << std::setfill('0') << std::setw(2) << d.day();
  return os << ss.str();
}
inline std::ostream& operator<<(std::ostream& os, const civil_hour& h) {
  std::stringstream ss;
  ss << civil_day(h) << 'T';
  ss << std::setfill('0') << std::setw(2) << h.hour();
  return os << ss.str();
}
inline std::ostream& operator<<(std::ostream& os, const civil_minute& m) {
  std::stringstream ss;
  ss << civil_hour(m) << ':';
  ss << std::setfill('0') << std::setw(2) << m.minute();
  return os << ss.str();
}
inline std::ostream& operator<<(std::ostream& os, const civil_second& s) {
  std::stringstream ss;
  ss << civil_minute(s) << ':';
  ss << std::setfill('0') << std::setw(2) << s.second();
  return os << ss.str();
}

////////////////////////////////////////////////////////////////////////

enum class weekday {
  monday,
  tuesday,
  wednesday,
  thursday,
  friday,
  saturday,
  sunday,
};

inline std::ostream& operator<<(std::ostream& os, weekday wd) {
  switch (wd) {
    case weekday::monday:
      return os << "Monday";
    case weekday::tuesday:
      return os << "Tuesday";
    case weekday::wednesday:
      return os << "Wednesday";
    case weekday::thursday:
      return os << "Thursday";
    case weekday::friday:
      return os << "Friday";
    case weekday::saturday:
      return os << "Saturday";
    case weekday::sunday:
      return os << "Sunday";
  }
}

CONSTEXPR_F weekday get_weekday(const civil_day& cd) noexcept {
  CONSTEXPR_D weekday k_weekday_by_thu_off[] = {
      weekday::thursday,  weekday::friday,  weekday::saturday,
      weekday::sunday,    weekday::monday,  weekday::tuesday,
      weekday::wednesday,
  };
  return k_weekday_by_thu_off[((cd - civil_day()) % 7 + 7) % 7];
}

////////////////////////////////////////////////////////////////////////

CONSTEXPR_F civil_day next_weekday(civil_day cd, weekday wd) noexcept {
  do { cd += 1; } while (get_weekday(cd) != wd);
  return cd;
}

CONSTEXPR_F civil_day prev_weekday(civil_day cd, weekday wd) noexcept {
  do { cd -= 1; } while (get_weekday(cd) != wd);
  return cd;
}

CONSTEXPR_F int get_yearday(const civil_day& cd) noexcept {
  return cd - civil_day(civil_year(cd)) + 1;
}

}  // namespace detail
}  // namespace cctz

#undef CONSTEXPR_T
#undef CONSTEXPR_M
#undef CONSTEXPR_F
#undef CONSTEXPR_D
