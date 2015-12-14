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

#include "src/cctz.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using std::chrono::system_clock;
using std::chrono::nanoseconds;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::minutes;
using std::chrono::hours;
using testing::HasSubstr;

namespace cctz {

namespace {

// This helper is a macro so that failed expectations show up with the
// correct line numbers.
#define ExpectTime(bd, y, m, d, hh, mm, ss, off, isdst, zone) \
  do {                                                        \
    EXPECT_EQ(y, bd.year);                                    \
    EXPECT_EQ(m, bd.month);                                   \
    EXPECT_EQ(d, bd.day);                                     \
    EXPECT_EQ(hh, bd.hour);                                   \
    EXPECT_EQ(mm, bd.minute);                                 \
    EXPECT_EQ(ss, bd.second);                                 \
    EXPECT_EQ(off, bd.offset);                                \
    EXPECT_EQ(isdst, bd.is_dst);                              \
    EXPECT_EQ(zone, bd.abbr);                                 \
  } while (0)

const char RFC3339_full[] = "%Y-%m-%dT%H:%M:%E*S%Ez";
const char RFC3339_sec[] =  "%Y-%m-%dT%H:%M:%S%Ez";

const char RFC1123_full[] = "%a, %d %b %Y %H:%M:%S %z";
const char RFC1123_no_wday[] =  "%d %b %Y %H:%M:%S %z";

// A helper that tests the given format specifier by itself, and with leading
// and trailing characters.  For example: TestFormatSpecifier(tp, "%a", "Thu").
template <typename D>
void TestFormatSpecifier(time_point<D> tp, TimeZone tz, const std::string& fmt,
                         const std::string& ans) {
  EXPECT_EQ(ans, Format(fmt, tp, tz));
  EXPECT_EQ("xxx " + ans, Format("xxx " + fmt, tp, tz));
  EXPECT_EQ(ans + " yyy", Format(fmt + " yyy", tp, tz));
  EXPECT_EQ("xxx " + ans + " yyy", Format("xxx " + fmt + " yyy", tp, tz));
}

}  // namespace

//
// Testing Format()
//

TEST(Format, TimePointResolution) {
  using std::chrono::time_point_cast;
  const char kFmt[] = "%H:%M:%E*S";
  const TimeZone utc = UTCTimeZone();
  const time_point<std::chrono::nanoseconds> t0 =
      system_clock::from_time_t(1420167845) + std::chrono::milliseconds(123) +
      std::chrono::microseconds(456) + std::chrono::nanoseconds(789);
  EXPECT_EQ("03:04:05.123456789",
            Format(kFmt, time_point_cast<std::chrono::nanoseconds>(t0), utc));
  EXPECT_EQ("03:04:05.123456",
            Format(kFmt, time_point_cast<std::chrono::microseconds>(t0), utc));
  EXPECT_EQ("03:04:05.123",
            Format(kFmt, time_point_cast<std::chrono::milliseconds>(t0), utc));
  EXPECT_EQ("03:04:05",
            Format(kFmt, time_point_cast<std::chrono::seconds>(t0), utc));
  EXPECT_EQ("03:04:05",
            Format(kFmt, time_point_cast<seconds64>(t0), utc));
  EXPECT_EQ("03:04:00",
            Format(kFmt, time_point_cast<std::chrono::minutes>(t0), utc));
  EXPECT_EQ("03:00:00",
            Format(kFmt, time_point_cast<std::chrono::hours>(t0), utc));
}

TEST(Format, Basics) {
  TimeZone tz = UTCTimeZone();
  time_point<std::chrono::nanoseconds> tp = system_clock::from_time_t(0);

  // Starts with a couple basic edge cases.
  EXPECT_EQ("", Format("", tp, tz));
  EXPECT_EQ(" ", Format(" ", tp, tz));
  EXPECT_EQ("  ", Format("  ", tp, tz));
  EXPECT_EQ("xxx", Format("xxx", tp, tz));
  std::string big(128, 'x');
  EXPECT_EQ(big, Format(big, tp, tz));
  // Cause the 1024-byte buffer to grow.
  std::string bigger(100000, 'x');
  EXPECT_EQ(bigger, Format(bigger, tp, tz));

  tp += hours(13) + minutes(4) + seconds(5);
  tp += milliseconds(6) + microseconds(7) + nanoseconds(8);
  EXPECT_EQ("1970-01-01", Format("%Y-%m-%d", tp, tz));
  EXPECT_EQ("13:04:05", Format("%H:%M:%S", tp, tz));
  EXPECT_EQ("13:04:05.006", Format("%H:%M:%E3S", tp, tz));
  EXPECT_EQ("13:04:05.006007", Format("%H:%M:%E6S", tp, tz));
  EXPECT_EQ("13:04:05.006007008", Format("%H:%M:%E9S", tp, tz));
}

TEST(Format, PosixConversions) {
  const TimeZone tz = UTCTimeZone();
  auto tp = system_clock::from_time_t(0);

  TestFormatSpecifier(tp, tz, "%d", "01");
  TestFormatSpecifier(tp, tz, "%e", " 1");  // extension but internal support
  TestFormatSpecifier(tp, tz, "%H", "00");
  TestFormatSpecifier(tp, tz, "%I", "12");
  TestFormatSpecifier(tp, tz, "%j", "001");
  TestFormatSpecifier(tp, tz, "%m", "01");
  TestFormatSpecifier(tp, tz, "%M", "00");
  TestFormatSpecifier(tp, tz, "%S", "00");
  TestFormatSpecifier(tp, tz, "%U", "00");
  TestFormatSpecifier(tp, tz, "%w", "4");  // 4=Thursday
  TestFormatSpecifier(tp, tz, "%W", "00");
  TestFormatSpecifier(tp, tz, "%y", "70");
  TestFormatSpecifier(tp, tz, "%Y", "1970");
  TestFormatSpecifier(tp, tz, "%z", "+0000");
  TestFormatSpecifier(tp, tz, "%Z", "UTC");
  TestFormatSpecifier(tp, tz, "%%", "%");

#if defined(__linux__)
  // SU/C99/TZ extensions
  TestFormatSpecifier(tp, tz, "%C", "19");
  TestFormatSpecifier(tp, tz, "%D", "01/01/70");
  TestFormatSpecifier(tp, tz, "%F", "1970-01-01");
  TestFormatSpecifier(tp, tz, "%g", "70");
  TestFormatSpecifier(tp, tz, "%G", "1970");
  TestFormatSpecifier(tp, tz, "%k", " 0");
  TestFormatSpecifier(tp, tz, "%l", "12");
  TestFormatSpecifier(tp, tz, "%n", "\n");
  TestFormatSpecifier(tp, tz, "%R", "00:00");
  TestFormatSpecifier(tp, tz, "%t", "\t");
  TestFormatSpecifier(tp, tz, "%T", "00:00:00");
  TestFormatSpecifier(tp, tz, "%u", "4");  // 4=Thursday
  TestFormatSpecifier(tp, tz, "%V", "01");
  TestFormatSpecifier(tp, tz, "%s", "0");
#endif
}

TEST(Format, LocaleSpecific) {
  const TimeZone tz = UTCTimeZone();
  auto tp = system_clock::from_time_t(0);

  TestFormatSpecifier(tp, tz, "%a", "Thu");
  TestFormatSpecifier(tp, tz, "%A", "Thursday");
  TestFormatSpecifier(tp, tz, "%b", "Jan");
  TestFormatSpecifier(tp, tz, "%B", "January");

  // %c should at least produce the numeric year and time-of-day.
  const std::string s = Format("%c", tp, UTCTimeZone());
  EXPECT_THAT(s, HasSubstr("1970"));
  EXPECT_THAT(s, HasSubstr("00:00:00"));

  TestFormatSpecifier(tp, tz, "%p", "AM");
  TestFormatSpecifier(tp, tz, "%x", "01/01/70");
  TestFormatSpecifier(tp, tz, "%X", "00:00:00");

#if defined(__linux__)
  // SU/C99/TZ extensions
  TestFormatSpecifier(tp, tz, "%h", "Jan");  // Same as %b
  TestFormatSpecifier(tp, tz, "%P", "am");
  TestFormatSpecifier(tp, tz, "%r", "12:00:00 AM");

  // Modified conversion specifiers %E_
  TestFormatSpecifier(tp, tz, "%Ec", "Thu Jan  1 00:00:00 1970");
  TestFormatSpecifier(tp, tz, "%EC", "19");
  TestFormatSpecifier(tp, tz, "%Ex", "01/01/70");
  TestFormatSpecifier(tp, tz, "%EX", "00:00:00");
  TestFormatSpecifier(tp, tz, "%Ey", "70");
  TestFormatSpecifier(tp, tz, "%EY", "1970");

  // Modified conversion specifiers %O_
  TestFormatSpecifier(tp, tz, "%Od", "01");
  TestFormatSpecifier(tp, tz, "%Oe", " 1");
  TestFormatSpecifier(tp, tz, "%OH", "00");
  TestFormatSpecifier(tp, tz, "%OI", "12");
  TestFormatSpecifier(tp, tz, "%Om", "01");
  TestFormatSpecifier(tp, tz, "%OM", "00");
  TestFormatSpecifier(tp, tz, "%OS", "00");
  TestFormatSpecifier(tp, tz, "%Ou", "4");  // 4=Thursday
  TestFormatSpecifier(tp, tz, "%OU", "00");
  TestFormatSpecifier(tp, tz, "%OV", "01");
  TestFormatSpecifier(tp, tz, "%Ow", "4");  // 4=Thursday
  TestFormatSpecifier(tp, tz, "%OW", "00");
  TestFormatSpecifier(tp, tz, "%Oy", "70");
#endif
}

TEST(Format, Escaping) {
  const TimeZone tz = UTCTimeZone();
  auto tp = system_clock::from_time_t(0);

  TestFormatSpecifier(tp, tz, "%%", "%");
  TestFormatSpecifier(tp, tz, "%%a", "%a");
  TestFormatSpecifier(tp, tz, "%%b", "%b");
  TestFormatSpecifier(tp, tz, "%%Ea", "%Ea");
  TestFormatSpecifier(tp, tz, "%%Es", "%Es");
  TestFormatSpecifier(tp, tz, "%%E3S", "%E3S");
  TestFormatSpecifier(tp, tz, "%%OS", "%OS");
  TestFormatSpecifier(tp, tz, "%%O3S", "%O3S");

  // Multiple levels of escaping.
  TestFormatSpecifier(tp, tz, "%%%Y", "%1970");
  TestFormatSpecifier(tp, tz, "%%%E3S", "%00.000");
  TestFormatSpecifier(tp, tz, "%%%%E3S", "%%E3S");
}

TEST(Format, ExtendedSeconds) {
  const TimeZone tz = UTCTimeZone();
  time_point<std::chrono::nanoseconds> tp = system_clock::from_time_t(0);
  tp += hours(3) + minutes(4) + seconds(5);
  tp += milliseconds(6) + microseconds(7) + nanoseconds(8);

  EXPECT_EQ("11045", Format("%s", tp, tz));

  EXPECT_EQ("03:04:05", Format("%H:%M:%E0S", tp, tz));
  EXPECT_EQ("03:04:05.0", Format("%H:%M:%E1S", tp, tz));
  EXPECT_EQ("03:04:05.00", Format("%H:%M:%E2S", tp, tz));
  EXPECT_EQ("03:04:05.006", Format("%H:%M:%E3S", tp, tz));
  EXPECT_EQ("03:04:05.0060", Format("%H:%M:%E4S", tp, tz));
  EXPECT_EQ("03:04:05.00600", Format("%H:%M:%E5S", tp, tz));
  EXPECT_EQ("03:04:05.006007", Format("%H:%M:%E6S", tp, tz));
  EXPECT_EQ("03:04:05.0060070", Format("%H:%M:%E7S", tp, tz));
  EXPECT_EQ("03:04:05.00600700", Format("%H:%M:%E8S", tp, tz));
  EXPECT_EQ("03:04:05.006007008", Format("%H:%M:%E9S", tp, tz));
  EXPECT_EQ("03:04:05.0060070080", Format("%H:%M:%E10S", tp, tz));
  EXPECT_EQ("03:04:05.00600700800", Format("%H:%M:%E11S", tp, tz));
  EXPECT_EQ("03:04:05.006007008000", Format("%H:%M:%E12S", tp, tz));
  EXPECT_EQ("03:04:05.0060070080000", Format("%H:%M:%E13S", tp, tz));
  EXPECT_EQ("03:04:05.00600700800000", Format("%H:%M:%E14S", tp, tz));
  EXPECT_EQ("03:04:05.006007008000000", Format("%H:%M:%E15S", tp, tz));

  EXPECT_EQ("03:04:05.006007008", Format("%H:%M:%E*S", tp, tz));

  // Times before the Unix epoch.
  tp = system_clock::from_time_t(0) + microseconds(-1);
  EXPECT_EQ("1969-12-31 23:59:59.999999",
            Format("%Y-%m-%d %H:%M:%E*S", tp, tz));

  // Here is a "%E*S" case we got wrong for a while.  While the first
  // instant below is correctly rendered as "...:07.333304", the second
  // one used to appear as "...:07.33330499999999999".
  tp = system_clock::from_time_t(0) + microseconds(1395024427333304);
  EXPECT_EQ("2014-03-17 02:47:07.333304",
            Format("%Y-%m-%d %H:%M:%E*S", tp, tz));
  tp += microseconds(1);
  EXPECT_EQ("2014-03-17 02:47:07.333305",
            Format("%Y-%m-%d %H:%M:%E*S", tp, tz));
}

TEST(Format, ExtendedOffset) {
  auto tp = system_clock::from_time_t(0);

  TimeZone tz = UTCTimeZone();
  TestFormatSpecifier(tp, tz, "%Ez", "+00:00");

  EXPECT_TRUE(LoadTimeZone("America/New_York", &tz));
  TestFormatSpecifier(tp, tz, "%Ez", "-05:00");

  EXPECT_TRUE(LoadTimeZone("America/Los_Angeles", &tz));
  TestFormatSpecifier(tp, tz, "%Ez", "-08:00");

  EXPECT_TRUE(LoadTimeZone("Australia/Sydney", &tz));
  TestFormatSpecifier(tp, tz, "%Ez", "+10:00");

  EXPECT_TRUE(LoadTimeZone("Africa/Monrovia", &tz));
  // The true offset is -00:44:30 but %z only gives (truncated) minutes.
  TestFormatSpecifier(tp, tz, "%z", "-0044");
  TestFormatSpecifier(tp, tz, "%Ez", "-00:44");
}

TEST(Format, ExtendedYears) {
  const TimeZone utc = UTCTimeZone();
  const char e4y_fmt[] = "%E4Y%m%d";  // no separators

  // %E4Y zero-pads the year to produce at least 4 chars, including the sign.
  auto tp = MakeTime(-999, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("-9991127", Format(e4y_fmt, tp, utc));
  tp = MakeTime(-99, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("-0991127", Format(e4y_fmt, tp, utc));
  tp = MakeTime(-9, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("-0091127", Format(e4y_fmt, tp, utc));
  tp = MakeTime(-1, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("-0011127", Format(e4y_fmt, tp, utc));
  tp = MakeTime(0, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("00001127", Format(e4y_fmt, tp, utc));
  tp = MakeTime(1, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("00011127", Format(e4y_fmt, tp, utc));
  tp = MakeTime(9, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("00091127", Format(e4y_fmt, tp, utc));
  tp = MakeTime(99, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("00991127", Format(e4y_fmt, tp, utc));
  tp = MakeTime(999, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("09991127", Format(e4y_fmt, tp, utc));
  tp = MakeTime(9999, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("99991127", Format(e4y_fmt, tp, utc));

  // When the year is outside [-999:9999], more than 4 chars are produced.
  tp = MakeTime(-1000, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("-10001127", Format(e4y_fmt, tp, utc));
  tp = MakeTime(10000, 11, 27, 0, 0, 0, utc);
  EXPECT_EQ("100001127", Format(e4y_fmt, tp, utc));
}

TEST(Format, RFC3339Format) {
  TimeZone tz;
  EXPECT_TRUE(LoadTimeZone("America/Los_Angeles", &tz));

  time_point<std::chrono::nanoseconds> tp = MakeTime(1977, 6, 28, 9, 8, 7, tz);
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_full, tp, tz));
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_sec, tp, tz));

  tp += milliseconds(100);
  EXPECT_EQ("1977-06-28T09:08:07.1-07:00", Format(RFC3339_full, tp, tz));
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_sec, tp, tz));

  tp += milliseconds(20);
  EXPECT_EQ("1977-06-28T09:08:07.12-07:00", Format(RFC3339_full, tp, tz));
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_sec, tp, tz));

  tp += milliseconds(3);
  EXPECT_EQ("1977-06-28T09:08:07.123-07:00", Format(RFC3339_full, tp, tz));
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_sec, tp, tz));

  tp += microseconds(400);
  EXPECT_EQ("1977-06-28T09:08:07.1234-07:00", Format(RFC3339_full, tp, tz));
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_sec, tp, tz));

  tp += microseconds(50);
  EXPECT_EQ("1977-06-28T09:08:07.12345-07:00", Format(RFC3339_full, tp, tz));
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_sec, tp, tz));

  tp += microseconds(6);
  EXPECT_EQ("1977-06-28T09:08:07.123456-07:00", Format(RFC3339_full, tp, tz));
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_sec, tp, tz));

  tp += nanoseconds(700);
  EXPECT_EQ("1977-06-28T09:08:07.1234567-07:00", Format(RFC3339_full, tp, tz));
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_sec, tp, tz));

  tp += nanoseconds(80);
  EXPECT_EQ("1977-06-28T09:08:07.12345678-07:00", Format(RFC3339_full, tp, tz));
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_sec, tp, tz));

  tp += nanoseconds(9);
  EXPECT_EQ("1977-06-28T09:08:07.123456789-07:00",
            Format(RFC3339_full, tp, tz));
  EXPECT_EQ("1977-06-28T09:08:07-07:00", Format(RFC3339_sec, tp, tz));
}

TEST(Format, RFC1123Format) {  // locale specific
  TimeZone tz;
  EXPECT_TRUE(LoadTimeZone("America/Los_Angeles", &tz));

  auto tp = MakeTime(1977, 6, 28, 9, 8, 7, tz);
  EXPECT_EQ("Tue, 28 Jun 1977 09:08:07 -0700", Format(RFC1123_full, tp, tz));
  EXPECT_EQ("28 Jun 1977 09:08:07 -0700", Format(RFC1123_no_wday, tp, tz));
}

//
// Testing Parse()
//

TEST(Parse, TimePointResolution) {
  using std::chrono::time_point_cast;
  const char kFmt[] = "%H:%M:%E*S";
  const TimeZone utc = UTCTimeZone();

  time_point<std::chrono::nanoseconds> tp_ns;
  EXPECT_TRUE(Parse(kFmt, "03:04:05.123456789", utc, &tp_ns));
  EXPECT_EQ("03:04:05.123456789", Format(kFmt, tp_ns, utc));
  EXPECT_TRUE(Parse(kFmt, "03:04:05.123456", utc, &tp_ns));
  EXPECT_EQ("03:04:05.123456", Format(kFmt, tp_ns, utc));

  time_point<std::chrono::microseconds> tp_us;
  EXPECT_TRUE(Parse(kFmt, "03:04:05.123456789", utc, &tp_us));
  EXPECT_EQ("03:04:05.123456", Format(kFmt, tp_us, utc));
  EXPECT_TRUE(Parse(kFmt, "03:04:05.123456", utc, &tp_us));
  EXPECT_EQ("03:04:05.123456", Format(kFmt, tp_us, utc));
  EXPECT_TRUE(Parse(kFmt, "03:04:05.123", utc, &tp_us));
  EXPECT_EQ("03:04:05.123", Format(kFmt, tp_us, utc));

  time_point<std::chrono::milliseconds> tp_ms;
  EXPECT_TRUE(Parse(kFmt, "03:04:05.123456", utc, &tp_ms));
  EXPECT_EQ("03:04:05.123", Format(kFmt, tp_ms, utc));
  EXPECT_TRUE(Parse(kFmt, "03:04:05.123", utc, &tp_ms));
  EXPECT_EQ("03:04:05.123", Format(kFmt, tp_ms, utc));
  EXPECT_TRUE(Parse(kFmt, "03:04:05", utc, &tp_ms));
  EXPECT_EQ("03:04:05", Format(kFmt, tp_ms, utc));

  time_point<std::chrono::seconds> tp_s;
  EXPECT_TRUE(Parse(kFmt, "03:04:05.123", utc, &tp_s));
  EXPECT_EQ("03:04:05", Format(kFmt, tp_s, utc));
  EXPECT_TRUE(Parse(kFmt, "03:04:05", utc, &tp_s));
  EXPECT_EQ("03:04:05", Format(kFmt, tp_s, utc));

  time_point<std::chrono::minutes> tp_m;
  EXPECT_TRUE(Parse(kFmt, "03:04:05", utc, &tp_m));
  EXPECT_EQ("03:04:00", Format(kFmt, tp_m, utc));

  time_point<std::chrono::hours> tp_h;
  EXPECT_TRUE(Parse(kFmt, "03:04:05", utc, &tp_h));
  EXPECT_EQ("03:00:00", Format(kFmt, tp_h, utc));
}

TEST(Parse, Basics) {
  TimeZone tz = UTCTimeZone();
  time_point<std::chrono::nanoseconds> tp =
      system_clock::from_time_t(1234567890);

  // Simple edge cases.
  EXPECT_TRUE(Parse("", "", tz, &tp));
  EXPECT_EQ(system_clock::from_time_t(0), tp);  // everything defaulted
  EXPECT_TRUE(Parse(" ", " ", tz, &tp));
  EXPECT_TRUE(Parse("  ", "  ", tz, &tp));
  EXPECT_TRUE(Parse("x", "x", tz, &tp));
  EXPECT_TRUE(Parse("xxx", "xxx", tz, &tp));

  EXPECT_TRUE(
      Parse("%Y-%m-%d %H:%M:%S %z", "2013-06-28 19:08:09 -0800", tz, &tp));
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 6, 29, 3, 8, 9, 0, false, "UTC");
}

TEST(Parse, WithTimeZone) {
  TimeZone tz;
  EXPECT_TRUE(LoadTimeZone("America/Los_Angeles", &tz));
  time_point<std::chrono::nanoseconds> tp;

  // We can parse a string without a UTC offset if we supply a timezone.
  EXPECT_TRUE(Parse("%Y-%m-%d %H:%M:%S", "2013-06-28 19:08:09", tz, &tp));
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 6, 28, 19, 8, 9, -7 * 60 * 60, true, "PDT");

  // But the timezone is ignored when a UTC offset is present.
  EXPECT_TRUE(
      Parse("%Y-%m-%d %H:%M:%S %z", "2013-06-28 19:08:09 +0800", tz, &tp));
  bd = BreakTime(tp, UTCTimeZone());
  ExpectTime(bd, 2013, 6, 28, 11, 8, 9, 0, false, "UTC");

  // Check a skipped time (a Spring DST transition).  Parse() returns
  // the preferred-offset result, as defined for ConvertDateTime().
  EXPECT_TRUE(Parse("%Y-%m-%d %H:%M:%S", "2011-03-13 02:15:00", tz, &tp));
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2011, 3, 13, 3, 15, 0, -7 * 60 * 60, true, "PDT");

  // Check a repeated time (a Fall DST transition).  Parse() returns
  // the preferred-offset result, as defined for ConvertDateTime().
  EXPECT_TRUE(Parse("%Y-%m-%d %H:%M:%S", "2011-11-06 01:15:00", tz, &tp));
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2011, 11, 6, 1, 15, 0, -7 * 60 * 60, true, "PDT");
}

TEST(Parse, LeapSecond) {
  TimeZone tz;
  EXPECT_TRUE(LoadTimeZone("America/Los_Angeles", &tz));
  time_point<std::chrono::nanoseconds> tp;

  // ":59" -> ":59"
  EXPECT_TRUE(Parse(RFC3339_full, "2013-06-28T07:08:59-08:00", tz, &tp));
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 6, 28, 8, 8, 59, -7 * 60 * 60, true, "PDT");

  // ":59.5" -> ":59.5"
  EXPECT_TRUE(Parse(RFC3339_full, "2013-06-28T07:08:59.5-08:00", tz, &tp));
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 6, 28, 8, 8, 59, -7 * 60 * 60, true, "PDT");

  // ":60" -> ":00"
  EXPECT_TRUE(Parse(RFC3339_full, "2013-06-28T07:08:60-08:00", tz, &tp));
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 6, 28, 8, 9, 0, -7 * 60 * 60, true, "PDT");

  // ":60.5" -> ":00.0"
  EXPECT_TRUE(Parse(RFC3339_full, "2013-06-28T07:08:60.5-08:00", tz, &tp));
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 6, 28, 8, 9, 0, -7 * 60 * 60, true, "PDT");

  // ":61" -> error
  EXPECT_FALSE(Parse(RFC3339_full, "2013-06-28T07:08:61-08:00", tz, &tp));
}

TEST(Parse, ErrorCases) {
  const TimeZone tz = UTCTimeZone();
  auto tp = system_clock::from_time_t(0);

  // Illegal trailing data.
  EXPECT_FALSE(Parse("%S", "123", tz, &tp));

  // Can't parse an illegal format specifier.
  EXPECT_FALSE(Parse("%Q", "x", tz, &tp));

  // Fails because of trailing, unparsed data "blah".
  EXPECT_FALSE(Parse("%m-%d", "2-3 blah", tz, &tp));

  // Trailing whitespace is allowed.
  EXPECT_TRUE(Parse("%m-%d", "2-3  ", tz, &tp));
  EXPECT_EQ(2, BreakTime(tp, UTCTimeZone()).month);
  EXPECT_EQ(3, BreakTime(tp, UTCTimeZone()).day);

  // Feb 31 requires normalization.
  EXPECT_FALSE(Parse("%m-%d", "2-31", tz, &tp));

  // Check that we cannot have spaces in UTC offsets.
  EXPECT_TRUE(Parse("%z", "-0203", tz, &tp));
  EXPECT_FALSE(Parse("%z", "- 2 3", tz, &tp));
  EXPECT_TRUE(Parse("%Ez", "-02:03", tz, &tp));
  EXPECT_FALSE(Parse("%Ez", "- 2: 3", tz, &tp));

  // Check that we reject other malformed UTC offsets.
  EXPECT_FALSE(Parse("%Ez", "+-08:00", tz, &tp));
  EXPECT_FALSE(Parse("%Ez", "-+08:00", tz, &tp));

  // Check that we do not accept "-0" in fields that allow zero.
  EXPECT_FALSE(Parse("%Y", "-0", tz, &tp));
  EXPECT_FALSE(Parse("%E4Y", "-0", tz, &tp));
  EXPECT_FALSE(Parse("%H", "-0", tz, &tp));
  EXPECT_FALSE(Parse("%M", "-0", tz, &tp));
  EXPECT_FALSE(Parse("%S", "-0", tz, &tp));
  EXPECT_FALSE(Parse("%z", "+-000", tz, &tp));
  EXPECT_FALSE(Parse("%Ez", "+-0:00", tz, &tp));
  EXPECT_FALSE(Parse("%z", "-00-0", tz, &tp));
  EXPECT_FALSE(Parse("%Ez", "-00:-0", tz, &tp));
}

TEST(Parse, PosixConversions) {
  TimeZone tz = UTCTimeZone();
  auto tp = system_clock::from_time_t(0);
  const auto reset = MakeTime(1977, 6, 28, 9, 8, 7, tz);

  tp = reset;
  EXPECT_TRUE(Parse("%d", "15", tz, &tp));
  EXPECT_EQ(15, BreakTime(tp, tz).day);

  // %e is an extension, but is supported internally.
  tp = reset;
  EXPECT_TRUE(Parse("%e", "15", tz, &tp));
  EXPECT_EQ(15, BreakTime(tp, tz).day);  // Equivalent to %d

  tp = reset;
  EXPECT_TRUE(Parse("%H", "17", tz, &tp));
  EXPECT_EQ(17, BreakTime(tp, tz).hour);

  tp = reset;
  EXPECT_TRUE(Parse("%I", "5", tz, &tp));
  EXPECT_EQ(5, BreakTime(tp, tz).hour);

  // %j is parsed but ignored.
  EXPECT_TRUE(Parse("%j", "32", tz, &tp));

  tp = reset;
  EXPECT_TRUE(Parse("%m", "11", tz, &tp));
  EXPECT_EQ(11, BreakTime(tp, tz).month);

  tp = reset;
  EXPECT_TRUE(Parse("%M", "33", tz, &tp));
  EXPECT_EQ(33, BreakTime(tp, tz).minute);

  tp = reset;
  EXPECT_TRUE(Parse("%S", "55", tz, &tp));
  EXPECT_EQ(55, BreakTime(tp, tz).second);

  // %U is parsed but ignored.
  EXPECT_TRUE(Parse("%U", "15", tz, &tp));

  // %w is parsed but ignored.
  EXPECT_TRUE(Parse("%w", "2", tz, &tp));

  // %W is parsed but ignored.
  EXPECT_TRUE(Parse("%W", "22", tz, &tp));

  tp = reset;
  EXPECT_TRUE(Parse("%y", "04", tz, &tp));
  EXPECT_EQ(2004, BreakTime(tp, tz).year);

  tp = reset;
  EXPECT_TRUE(Parse("%Y", "2004", tz, &tp));
  EXPECT_EQ(2004, BreakTime(tp, tz).year);

  EXPECT_TRUE(Parse("%%", "%", tz, &tp));

#if defined(__linux__)
  // SU/C99/TZ extensions

  tp = reset;
  EXPECT_TRUE(Parse("%C", "20", tz, &tp));
  EXPECT_EQ(2000, BreakTime(tp, tz).year);

  tp = reset;
  EXPECT_TRUE(Parse("%D", "02/03/04", tz, &tp));
  EXPECT_EQ(2, BreakTime(tp, tz).month);
  EXPECT_EQ(3, BreakTime(tp, tz).day);
  EXPECT_EQ(2004, BreakTime(tp, tz).year);

  EXPECT_TRUE(Parse("%n", "\n", tz, &tp));

  tp = reset;
  EXPECT_TRUE(Parse("%R", "03:44", tz, &tp));
  EXPECT_EQ(3, BreakTime(tp, tz).hour);
  EXPECT_EQ(44, BreakTime(tp, tz).minute);

  EXPECT_TRUE(Parse("%t", "\t\v\f\n\r ", tz, &tp));

  tp = reset;
  EXPECT_TRUE(Parse("%T", "03:44:55", tz, &tp));
  EXPECT_EQ(3, BreakTime(tp, tz).hour);
  EXPECT_EQ(44, BreakTime(tp, tz).minute);
  EXPECT_EQ(55, BreakTime(tp, tz).second);

  tp = reset;
  EXPECT_TRUE(Parse("%s", "1234567890", tz, &tp));
  EXPECT_EQ(system_clock::from_time_t(1234567890), tp);

  // %s conversion, like %z/%Ez, pays no heed to the optional zone.
  TimeZone lax;
  EXPECT_TRUE(LoadTimeZone("America/Los_Angeles", &lax));
  tp = reset;
  EXPECT_TRUE(Parse("%s", "1234567890", lax, &tp));
  EXPECT_EQ(system_clock::from_time_t(1234567890), tp);

  // This is most important when the time has the same YMDhms
  // breakdown in the zone as some other time.  For example, ...
  //  1414917000 in US/Pacific -> Sun Nov 2 01:30:00 2014 (PDT)
  //  1414920600 in US/Pacific -> Sun Nov 2 01:30:00 2014 (PST)
  tp = reset;
  EXPECT_TRUE(Parse("%s", "1414917000", lax, &tp));
  EXPECT_EQ(system_clock::from_time_t(1414917000), tp);
  tp = reset;
  EXPECT_TRUE(Parse("%s", "1414920600", lax, &tp));
  EXPECT_EQ(system_clock::from_time_t(1414920600), tp);
#endif
}

TEST(Parse, LocaleSpecific) {
  TimeZone tz = UTCTimeZone();
  auto tp = system_clock::from_time_t(0);
  const auto reset = MakeTime(1977, 6, 28, 9, 8, 7, tz);

  // %a is parsed but ignored.
  EXPECT_TRUE(Parse("%a", "Mon", tz, &tp));

  // %A is parsed but ignored.
  EXPECT_TRUE(Parse("%A", "Monday", tz, &tp));

  tp = reset;
  EXPECT_TRUE(Parse("%b", "Feb", tz, &tp));
  EXPECT_EQ(2, BreakTime(tp, tz).month);

  tp = reset;
  EXPECT_TRUE(Parse("%B", "February", tz, &tp));
  EXPECT_EQ(2, BreakTime(tp, tz).month);

  // %p is parsed but ignored if it's alone.  But it's used with %I.
  EXPECT_TRUE(Parse("%p", "AM", tz, &tp));
  tp = reset;
  EXPECT_TRUE(Parse("%I %p", "5 PM", tz, &tp));
  EXPECT_EQ(17, BreakTime(tp, tz).hour);

  tp = reset;
  EXPECT_TRUE(Parse("%x", "02/03/04", tz, &tp));
  EXPECT_EQ(2, BreakTime(tp, tz).month);
  EXPECT_EQ(3, BreakTime(tp, tz).day);
  EXPECT_EQ(2004, BreakTime(tp, tz).year);

  tp = reset;
  EXPECT_TRUE(Parse("%X", "15:44:55", tz, &tp));
  EXPECT_EQ(15, BreakTime(tp, tz).hour);
  EXPECT_EQ(44, BreakTime(tp, tz).minute);
  EXPECT_EQ(55, BreakTime(tp, tz).second);

#if defined(__linux__)
  // SU/C99/TZ extensions

  tp = reset;
  EXPECT_TRUE(Parse("%h", "Feb", tz, &tp));
  EXPECT_EQ(2, BreakTime(tp, tz).month);  // Equivalent to %b

  tp = reset;
  EXPECT_TRUE(Parse("%l %p", "5 PM", tz, &tp));
  EXPECT_EQ(17, BreakTime(tp, tz).hour);

  tp = reset;
  EXPECT_TRUE(Parse("%r", "03:44:55 PM", tz, &tp));
  EXPECT_EQ(15, BreakTime(tp, tz).hour);
  EXPECT_EQ(44, BreakTime(tp, tz).minute);
  EXPECT_EQ(55, BreakTime(tp, tz).second);

  tp = reset;
  EXPECT_TRUE(Parse("%Ec", "Tue Nov 19 05:06:07 2013", tz, &tp));
  EXPECT_EQ(MakeTime(2013, 11, 19, 5, 6, 7, tz), tp);

  // Modified conversion specifiers %E_

  tp = reset;
  EXPECT_TRUE(Parse("%EC", "20", tz, &tp));
  EXPECT_EQ(2000, BreakTime(tp, tz).year);

  tp = reset;
  EXPECT_TRUE(Parse("%Ex", "02/03/04", tz, &tp));
  EXPECT_EQ(2, BreakTime(tp, tz).month);
  EXPECT_EQ(3, BreakTime(tp, tz).day);
  EXPECT_EQ(2004, BreakTime(tp, tz).year);

  tp = reset;
  EXPECT_TRUE(Parse("%EX", "15:44:55", tz, &tp));
  EXPECT_EQ(15, BreakTime(tp, tz).hour);
  EXPECT_EQ(44, BreakTime(tp, tz).minute);
  EXPECT_EQ(55, BreakTime(tp, tz).second);

// %Ey, the year offset from %EC, doesn't really make sense alone as there
// is no way to represent it in tm_year (%EC is not simply the century).
// Yet, because we handle each (non-internal) specifier in a separate call
// to strptime(), there is no way to group %EC and %Ey either.  So we just
// skip the %Ey case.
#if 0
  tp = reset;
  EXPECT_TRUE(Parse("%Ey", "04", tz, &tp));
  EXPECT_EQ(2004, BreakTime(tp, tz).year);
#endif

  tp = reset;
  EXPECT_TRUE(Parse("%EY", "2004", tz, &tp));
  EXPECT_EQ(2004, BreakTime(tp, tz).year);

  // Modified conversion specifiers %O_

  tp = reset;
  EXPECT_TRUE(Parse("%Od", "15", tz, &tp));
  EXPECT_EQ(15, BreakTime(tp, tz).day);

  tp = reset;
  EXPECT_TRUE(Parse("%Oe", "15", tz, &tp));
  EXPECT_EQ(15, BreakTime(tp, tz).day);  // Equivalent to %d

  tp = reset;
  EXPECT_TRUE(Parse("%OH", "17", tz, &tp));
  EXPECT_EQ(17, BreakTime(tp, tz).hour);

  tp = reset;
  EXPECT_TRUE(Parse("%OI", "5", tz, &tp));
  EXPECT_EQ(5, BreakTime(tp, tz).hour);

  tp = reset;
  EXPECT_TRUE(Parse("%Om", "11", tz, &tp));
  EXPECT_EQ(11, BreakTime(tp, tz).month);

  tp = reset;
  EXPECT_TRUE(Parse("%OM", "33", tz, &tp));
  EXPECT_EQ(33, BreakTime(tp, tz).minute);

  tp = reset;
  EXPECT_TRUE(Parse("%OS", "55", tz, &tp));
  EXPECT_EQ(55, BreakTime(tp, tz).second);

  // %OU is parsed but ignored.
  EXPECT_TRUE(Parse("%OU", "15", tz, &tp));

  // %Ow is parsed but ignored.
  EXPECT_TRUE(Parse("%Ow", "2", tz, &tp));

  // %OW is parsed but ignored.
  EXPECT_TRUE(Parse("%OW", "22", tz, &tp));

  tp = reset;
  EXPECT_TRUE(Parse("%Oy", "04", tz, &tp));
  EXPECT_EQ(2004, BreakTime(tp, tz).year);
#endif
}

TEST(Parse, ExtendedSeconds) {
  const TimeZone tz = UTCTimeZone();

  // Here is a "%E*S" case we got wrong for a while.  The fractional
  // part of the first instant is less than 2^31 and was correctly
  // parsed, while the second (and any subsecond field >=2^31) failed.
  time_point<std::chrono::nanoseconds> tp = system_clock::from_time_t(0);
  EXPECT_TRUE(Parse("%E*S", "0.2147483647", tz, &tp));
  EXPECT_EQ(system_clock::from_time_t(0) + nanoseconds(214748364), tp);
  tp = system_clock::from_time_t(0);
  EXPECT_TRUE(Parse("%E*S", "0.2147483648", tz, &tp));
  EXPECT_EQ(system_clock::from_time_t(0) + nanoseconds(214748364), tp);

  // We should also be able to specify long strings of digits far
  // beyond the current resolution and have them convert the same way.
  tp = system_clock::from_time_t(0);
  EXPECT_TRUE(Parse(
      "%E*S", "0.214748364801234567890123456789012345678901234567890123456789",
      tz, &tp));
  EXPECT_EQ(system_clock::from_time_t(0) + nanoseconds(214748364), tp);
}

TEST(Parse, ExtendedSecondsScan) {
  const TimeZone tz = UTCTimeZone();
  time_point<std::chrono::nanoseconds> tp;
  for (int64_t ms = 0; ms < 1000; ms += 111) {
    for (int64_t us = 0; us < 1000; us += 27) {
      const int64_t micros = ms * 1000 + us;
      for (int64_t ns = 0; ns < 1000; ns += 9) {
        const auto expected =
            system_clock::from_time_t(0) + nanoseconds(micros * 1000 + ns);
        std::ostringstream oss;
        oss << "0." << std::setfill('0') << std::setw(3);
        oss << ms << std::setw(3) << us << std::setw(3) << ns;
        const std::string input = oss.str();
        EXPECT_TRUE(Parse("%E*S", input, tz, &tp));
        EXPECT_EQ(expected, tp) << input;
      }
    }
  }
}

TEST(Parse, ExtendedOffset) {
  const TimeZone utc = UTCTimeZone();
  time_point<seconds64> tp;

  // %z against +-HHMM.
  EXPECT_TRUE(Parse("%z", "+0000", utc, &tp));
  EXPECT_EQ(MakeTime(1970, 1, 1, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse("%z", "-1234", utc, &tp));
  EXPECT_EQ(MakeTime(1970, 1, 1, 12, 34, 0, utc), tp);
  EXPECT_TRUE(Parse("%z", "+1234", utc, &tp));
  EXPECT_EQ(MakeTime(1969, 12, 31, 11, 26, 0, utc), tp);
  EXPECT_FALSE(Parse("%z", "-123", utc, &tp));

  // %z against +-HH.
  EXPECT_TRUE(Parse("%z", "+00", utc, &tp));
  EXPECT_EQ(MakeTime(1970, 1, 1, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse("%z", "-12", utc, &tp));
  EXPECT_EQ(MakeTime(1970, 1, 1, 12, 0, 0, utc), tp);
  EXPECT_TRUE(Parse("%z", "+12", utc, &tp));
  EXPECT_EQ(MakeTime(1969, 12, 31, 12, 0, 0, utc), tp);
  EXPECT_FALSE(Parse("%z", "-1", utc, &tp));

  // %Ez against +-HH:MM.
  EXPECT_TRUE(Parse("%Ez", "+00:00", utc, &tp));
  EXPECT_EQ(MakeTime(1970, 1, 1, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse("%Ez", "-12:34", utc, &tp));
  EXPECT_EQ(MakeTime(1970, 1, 1, 12, 34, 0, utc), tp);
  EXPECT_TRUE(Parse("%Ez", "+12:34", utc, &tp));
  EXPECT_EQ(MakeTime(1969, 12, 31, 11, 26, 0, utc), tp);
  EXPECT_FALSE(Parse("%Ez", "-12:3", utc, &tp));

  // %Ez against +-HHMM.
  EXPECT_TRUE(Parse("%Ez", "+0000", utc, &tp));
  EXPECT_EQ(MakeTime(1970, 1, 1, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse("%Ez", "-1234", utc, &tp));
  EXPECT_EQ(MakeTime(1970, 1, 1, 12, 34, 0, utc), tp);
  EXPECT_TRUE(Parse("%Ez", "+1234", utc, &tp));
  EXPECT_EQ(MakeTime(1969, 12, 31, 11, 26, 0, utc), tp);
  EXPECT_FALSE(Parse("%Ez", "-123", utc, &tp));

  // %Ez against +-HH.
  EXPECT_TRUE(Parse("%Ez", "+00", utc, &tp));
  EXPECT_EQ(MakeTime(1970, 1, 1, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse("%Ez", "-12", utc, &tp));
  EXPECT_EQ(MakeTime(1970, 1, 1, 12, 0, 0, utc), tp);
  EXPECT_TRUE(Parse("%Ez", "+12", utc, &tp));
  EXPECT_EQ(MakeTime(1969, 12, 31, 12, 0, 0, utc), tp);
  EXPECT_FALSE(Parse("%Ez", "-1", utc, &tp));
}

TEST(Parse, ExtendedYears) {
  const TimeZone utc = UTCTimeZone();
  const char e4y_fmt[] = "%E4Y%m%d";  // no separators
  time_point<seconds64> tp;

  // %E4Y consumes exactly four chars, including any sign.
  EXPECT_TRUE(Parse(e4y_fmt, "-9991127", utc, &tp));
  EXPECT_EQ(MakeTime(-999, 11, 27, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse(e4y_fmt, "-0991127", utc, &tp));
  EXPECT_EQ(MakeTime(-99, 11, 27, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse(e4y_fmt, "-0091127", utc, &tp));
  EXPECT_EQ(MakeTime(-9, 11, 27, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse(e4y_fmt, "-0011127", utc, &tp));
  EXPECT_EQ(MakeTime(-1, 11, 27, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse(e4y_fmt, "00001127", utc, &tp));
  EXPECT_EQ(MakeTime(0, 11, 27, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse(e4y_fmt, "00011127", utc, &tp));
  EXPECT_EQ(MakeTime(1, 11, 27, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse(e4y_fmt, "00091127", utc, &tp));
  EXPECT_EQ(MakeTime(9, 11, 27, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse(e4y_fmt, "00991127", utc, &tp));
  EXPECT_EQ(MakeTime(99, 11, 27, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse(e4y_fmt, "09991127", utc, &tp));
  EXPECT_EQ(MakeTime(999, 11, 27, 0, 0, 0, utc), tp);
  EXPECT_TRUE(Parse(e4y_fmt, "99991127", utc, &tp));
  EXPECT_EQ(MakeTime(9999, 11, 27, 0, 0, 0, utc), tp);

  // When the year is outside [-999:9999], the parse fails.
  EXPECT_FALSE(Parse(e4y_fmt, "-10001127", utc, &tp));
  EXPECT_FALSE(Parse(e4y_fmt, "100001127", utc, &tp));
}

TEST(Parse, RFC3339Format) {
  const TimeZone tz = UTCTimeZone();
  time_point<std::chrono::nanoseconds> tp;
  EXPECT_TRUE(Parse(RFC3339_sec, "2014-02-12T20:21:00+00:00", tz, &tp));
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2014, 2, 12, 20, 21, 0, 0, false, "UTC");

  // Check that %Ez also accepts "Z" as a synonym for "+00:00".
  time_point<std::chrono::nanoseconds> tp2;
  EXPECT_TRUE(Parse(RFC3339_sec, "2014-02-12T20:21:00Z", tz, &tp2));
  EXPECT_EQ(tp, tp2);
}

//
// Roundtrip test for Format()/Parse().
//

TEST(FormatParse, RoundTrip) {
  TimeZone lax;
  EXPECT_TRUE(LoadTimeZone("America/Los_Angeles", &lax));
  const auto in = MakeTime(1977, 6, 28, 9, 8, 7, lax);
  const auto subseconds = nanoseconds(654321);

  // RFC3339, which renders subseconds.
  {
    time_point<std::chrono::nanoseconds> out;
    const std::string s = Format(RFC3339_full, in + subseconds, lax);
    EXPECT_TRUE(Parse(RFC3339_full, s, lax, &out)) << s;
    EXPECT_EQ(in + subseconds, out);  // RFC3339_full includes %Ez
  }

  // RFC1123, which only does whole seconds.
  {
    time_point<std::chrono::nanoseconds> out;
    const std::string s = Format(RFC1123_full, in, lax);
    EXPECT_TRUE(Parse(RFC1123_full, s, lax, &out)) << s;
    EXPECT_EQ(in, out);  // RFC1123_full includes %z
  }

  // Even though we don't know what %c will produce, it should roundtrip,
  // but only in the 0-offset timezone.
  {
    time_point<std::chrono::nanoseconds> out;
    TimeZone utc = UTCTimeZone();
    const std::string s = Format("%c", in, utc);
    EXPECT_TRUE(Parse("%c", s, utc, &out)) << s;
    EXPECT_EQ(in, out);
  }
}

}  // namespace cctz
