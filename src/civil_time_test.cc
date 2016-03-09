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

#include "civil_time.h"

#include <cstdio>
#include <limits>
#include <string>

#include "gtest/gtest.h"

namespace cctz {

namespace {

std::string Format(const civil_second& cs) {
  char buf[sizeof "-2147483648-12-31T23:59:59"];
  std::snprintf(buf, sizeof buf, "%d-%02d-%02dT%02d:%02d:%02d", cs.year(),
                cs.month(), cs.day(), cs.hour(), cs.minute(), cs.second());
  return std::string(buf);
}

std::string Format(const civil_minute& cm) {
  char buf[sizeof "-2147483648-12-31T23:59"];
  std::snprintf(buf, sizeof buf, "%d-%02d-%02dT%02d:%02d", cm.year(),
                cm.month(), cm.day(), cm.hour(), cm.minute());
  return std::string(buf);
}

std::string Format(const civil_hour& ch) {
  char buf[sizeof "-2147483648-12-31T23"];
  std::snprintf(buf, sizeof buf, "%d-%02d-%02dT%02d", ch.year(), ch.month(),
                ch.day(), ch.hour());
  return std::string(buf);
}

std::string Format(const civil_day& cd) {
  char buf[sizeof "-2147483648-12-31"];
  std::snprintf(buf, sizeof buf, "%d-%02d-%02d", cd.year(), cd.month(),
                cd.day());
  return std::string(buf);
}

std::string Format(const civil_month& cm) {
  char buf[sizeof "-2147483648-12"];
  std::snprintf(buf, sizeof buf, "%d-%02d", cm.year(), cm.month());
  return std::string(buf);
}

std::string Format(const civil_year& cy) {
  char buf[sizeof "-2147483648"];
  std::snprintf(buf, sizeof buf, "%d", cy.year());
  return std::string(buf);
}

}  // namespace

// Construction tests

TEST(CivilTime, Normal) {
  constexpr civil_second css(2016, 1, 28, 17, 14, 12);
  constexpr civil_minute cmm(2016, 1, 28, 17, 14);
  constexpr civil_hour chh(2016, 1, 28, 17);
  constexpr civil_day cd(2016, 1, 28);
  constexpr civil_month cm(2016, 1);
  constexpr civil_year cy(2016);
}

TEST(CivilTime, Conversion) {
  constexpr civil_year cy(2016);
  constexpr civil_month cm(cy);
  constexpr civil_day cd(cm);
  constexpr civil_hour chh(cd);
  constexpr civil_minute cmm(chh);
  constexpr civil_second css(cmm);
}

// Normalization tests

TEST(CivilTime, Normalized) {
  constexpr civil_second cs(2016, 1, 28, 17, 14, 12);
  EXPECT_EQ(2016, cs.year());
  EXPECT_EQ(1, cs.month());
  EXPECT_EQ(28, cs.day());
  EXPECT_EQ(17, cs.hour());
  EXPECT_EQ(14, cs.minute());
  EXPECT_EQ(12, cs.second());
}

TEST(CivilTime, SecondOverflow) {
  constexpr civil_second cs(2016, 1, 28, 17, 14, 121);
  EXPECT_EQ(2016, cs.year());
  EXPECT_EQ(1, cs.month());
  EXPECT_EQ(28, cs.day());
  EXPECT_EQ(17, cs.hour());
  EXPECT_EQ(16, cs.minute());
  EXPECT_EQ(1, cs.second());
}

TEST(CivilTime, SecondUnderflow) {
  constexpr civil_second cs(2016, 1, 28, 17, 14, -121);
  EXPECT_EQ(2016, cs.year());
  EXPECT_EQ(1, cs.month());
  EXPECT_EQ(28, cs.day());
  EXPECT_EQ(17, cs.hour());
  EXPECT_EQ(11, cs.minute());
  EXPECT_EQ(59, cs.second());
}

TEST(CivilTime, MinuteOverflow) {
  constexpr civil_second cs(2016, 1, 28, 17, 121, 12);
  EXPECT_EQ(2016, cs.year());
  EXPECT_EQ(1, cs.month());
  EXPECT_EQ(28, cs.day());
  EXPECT_EQ(19, cs.hour());
  EXPECT_EQ(1, cs.minute());
  EXPECT_EQ(12, cs.second());
}

TEST(CivilTime, MinuteUnderflow) {
  constexpr civil_second cs(2016, 1, 28, 17, -121, 12);
  EXPECT_EQ(2016, cs.year());
  EXPECT_EQ(1, cs.month());
  EXPECT_EQ(28, cs.day());
  EXPECT_EQ(14, cs.hour());
  EXPECT_EQ(59, cs.minute());
  EXPECT_EQ(12, cs.second());
}

TEST(CivilTime, HourOverflow) {
  constexpr civil_second cs(2016, 1, 28, 49, 14, 12);
  EXPECT_EQ(2016, cs.year());
  EXPECT_EQ(1, cs.month());
  EXPECT_EQ(30, cs.day());
  EXPECT_EQ(1, cs.hour());
  EXPECT_EQ(14, cs.minute());
  EXPECT_EQ(12, cs.second());
}

TEST(CivilTime, HourUnderflow) {
  constexpr civil_second cs(2016, 1, 28, -49, 14, 12);
  EXPECT_EQ(2016, cs.year());
  EXPECT_EQ(1, cs.month());
  EXPECT_EQ(25, cs.day());
  EXPECT_EQ(23, cs.hour());
  EXPECT_EQ(14, cs.minute());
  EXPECT_EQ(12, cs.second());
}

TEST(CivilTime, MonthOverflow) {
  constexpr civil_second cs(2016, 25, 28, 17, 14, 12);
  EXPECT_EQ(2018, cs.year());
  EXPECT_EQ(1, cs.month());
  EXPECT_EQ(28, cs.day());
  EXPECT_EQ(17, cs.hour());
  EXPECT_EQ(14, cs.minute());
  EXPECT_EQ(12, cs.second());
}

TEST(CivilTime, MonthUnderflow) {
  constexpr civil_second cs(2016, -25, 28, 17, 14, 12);
  EXPECT_EQ(2013, cs.year());
  EXPECT_EQ(11, cs.month());
  EXPECT_EQ(28, cs.day());
  EXPECT_EQ(17, cs.hour());
  EXPECT_EQ(14, cs.minute());
  EXPECT_EQ(12, cs.second());
}

TEST(CivilTime, C4Overflow) {
  constexpr civil_second cs(2016, 1, 292195, 17, 14, 12);
  EXPECT_EQ(2816, cs.year());
  EXPECT_EQ(1, cs.month());
  EXPECT_EQ(1, cs.day());
  EXPECT_EQ(17, cs.hour());
  EXPECT_EQ(14, cs.minute());
  EXPECT_EQ(12, cs.second());
}

TEST(CivilTime, C4Underflow) {
  constexpr civil_second cs(2016, 1, -292195, 17, 14, 12);
  EXPECT_EQ(1215, cs.year());
  EXPECT_EQ(12, cs.month());
  EXPECT_EQ(30, cs.day());
  EXPECT_EQ(17, cs.hour());
  EXPECT_EQ(14, cs.minute());
  EXPECT_EQ(12, cs.second());
}

TEST(CivilTime, MixedNormalization) {
  constexpr civil_second cs(2016, -42, 122, 99, -147, 4949);
  EXPECT_EQ(2012, cs.year());
  EXPECT_EQ(10, cs.month());
  EXPECT_EQ(4, cs.day());
  EXPECT_EQ(1, cs.hour());
  EXPECT_EQ(55, cs.minute());
  EXPECT_EQ(29, cs.second());
}

// Relational tests

TEST(CivilTime, Less) {
  constexpr civil_second cs1(2016, 1, 28, 17, 14, 12);
  constexpr civil_second cs2(2016, 1, 28, 17, 14, 13);
  constexpr bool less = cs1 < cs2;
  EXPECT_TRUE(less);
}

// Arithmetic tests

TEST(CivilTime, Addition) {
  constexpr civil_second cs1(2016, 1, 28, 17, 14, 12);
  constexpr civil_second cs2 = cs1 + 50;
  EXPECT_EQ(2016, cs2.year());
  EXPECT_EQ(1, cs2.month());
  EXPECT_EQ(28, cs2.day());
  EXPECT_EQ(17, cs2.hour());
  EXPECT_EQ(15, cs2.minute());
  EXPECT_EQ(2, cs2.second());
}

TEST(CivilTime, Subtraction) {
  constexpr civil_second cs1(2016, 1, 28, 17, 14, 12);
  constexpr civil_second cs2 = cs1 - 50;
  EXPECT_EQ(2016, cs2.year());
  EXPECT_EQ(1, cs2.month());
  EXPECT_EQ(28, cs2.day());
  EXPECT_EQ(17, cs2.hour());
  EXPECT_EQ(13, cs2.minute());
  EXPECT_EQ(22, cs2.second());
}

TEST(CivilTime, Diff) {
  constexpr civil_day cd1(2016, 1, 28);
  constexpr civil_day cd2(2015, 1, 28);
  constexpr int diff = cd1 - cd2;
  EXPECT_EQ(365, diff);
}

// Helper tests

TEST(CivilTime, WeekDay) {
  constexpr civil_day cd(2016, 1, 28);
  constexpr weekday wd = get_weekday(cd);
  EXPECT_EQ(weekday::thursday, wd);
}

TEST(CivilTime, NextWeekDay) {
  constexpr civil_day cd(2016, 1, 28);
  constexpr civil_day next = next_weekday(cd, weekday::thursday);
  EXPECT_EQ(2016, next.year());
  EXPECT_EQ(2, next.month());
  EXPECT_EQ(4, next.day());
}

TEST(CivilTime, PrevWeekDay) {
  constexpr civil_day cd(2016, 1, 28);
  constexpr civil_day prev = prev_weekday(cd, weekday::thursday);
  EXPECT_EQ(2016, prev.year());
  EXPECT_EQ(1, prev.month());
  EXPECT_EQ(21, prev.day());
}

TEST(CivilTime, YearDay) {
  constexpr civil_day cd(2016, 1, 28);
  constexpr int yd = get_yearday(cd);
  EXPECT_EQ(28, yd);
}

// START OF google3 TESTS

TEST(CivilTime, DefaultConstruction) {
  civil_second ss;
  EXPECT_EQ("1970-01-01T00:00:00", Format(ss));

  civil_minute mm;
  EXPECT_EQ("1970-01-01T00:00", Format(mm));

  civil_hour hh;
  EXPECT_EQ("1970-01-01T00", Format(hh));

  civil_day d;
  EXPECT_EQ("1970-01-01", Format(d));

  civil_month m;
  EXPECT_EQ("1970-01", Format(m));

  civil_year y;
  EXPECT_EQ("1970", Format(y));
}

TEST(CivilTime, StructMember) {
  struct S {
    civil_day day;
  };
  S s = {};
  EXPECT_EQ(civil_day{}, s.day);
}

TEST(CivilTime, FieldsConstruction) {
  EXPECT_EQ("2015-01-02T03:04:05", Format(civil_second(2015, 1, 2, 3, 4, 5)));
  EXPECT_EQ("2015-01-02T03:04:00", Format(civil_second(2015, 1, 2, 3, 4)));
  EXPECT_EQ("2015-01-02T03:00:00", Format(civil_second(2015, 1, 2, 3)));
  EXPECT_EQ("2015-01-02T00:00:00", Format(civil_second(2015, 1, 2)));
  EXPECT_EQ("2015-01-01T00:00:00", Format(civil_second(2015, 1)));
  EXPECT_EQ("2015-01-01T00:00:00", Format(civil_second(2015)));

  EXPECT_EQ("2015-01-02T03:04", Format(civil_minute(2015, 1, 2, 3, 4, 5)));
  EXPECT_EQ("2015-01-02T03:04", Format(civil_minute(2015, 1, 2, 3, 4)));
  EXPECT_EQ("2015-01-02T03:00", Format(civil_minute(2015, 1, 2, 3)));
  EXPECT_EQ("2015-01-02T00:00", Format(civil_minute(2015, 1, 2)));
  EXPECT_EQ("2015-01-01T00:00", Format(civil_minute(2015, 1)));
  EXPECT_EQ("2015-01-01T00:00", Format(civil_minute(2015)));

  EXPECT_EQ("2015-01-02T03", Format(civil_hour(2015, 1, 2, 3, 4, 5)));
  EXPECT_EQ("2015-01-02T03", Format(civil_hour(2015, 1, 2, 3, 4)));
  EXPECT_EQ("2015-01-02T03", Format(civil_hour(2015, 1, 2, 3)));
  EXPECT_EQ("2015-01-02T00", Format(civil_hour(2015, 1, 2)));
  EXPECT_EQ("2015-01-01T00", Format(civil_hour(2015, 1)));
  EXPECT_EQ("2015-01-01T00", Format(civil_hour(2015)));

  EXPECT_EQ("2015-01-02", Format(civil_day(2015, 1, 2, 3, 4, 5)));
  EXPECT_EQ("2015-01-02", Format(civil_day(2015, 1, 2, 3, 4)));
  EXPECT_EQ("2015-01-02", Format(civil_day(2015, 1, 2, 3)));
  EXPECT_EQ("2015-01-02", Format(civil_day(2015, 1, 2)));
  EXPECT_EQ("2015-01-01", Format(civil_day(2015, 1)));
  EXPECT_EQ("2015-01-01", Format(civil_day(2015)));

  EXPECT_EQ("2015-01", Format(civil_month(2015, 1, 2, 3, 4, 5)));
  EXPECT_EQ("2015-01", Format(civil_month(2015, 1, 2, 3, 4)));
  EXPECT_EQ("2015-01", Format(civil_month(2015, 1, 2, 3)));
  EXPECT_EQ("2015-01", Format(civil_month(2015, 1, 2)));
  EXPECT_EQ("2015-01", Format(civil_month(2015, 1)));
  EXPECT_EQ("2015-01", Format(civil_month(2015)));

  EXPECT_EQ("2015", Format(civil_year(2015, 1, 2, 3, 4, 5)));
  EXPECT_EQ("2015", Format(civil_year(2015, 1, 2, 3, 4)));
  EXPECT_EQ("2015", Format(civil_year(2015, 1, 2, 3)));
  EXPECT_EQ("2015", Format(civil_year(2015, 1, 2)));
  EXPECT_EQ("2015", Format(civil_year(2015, 1)));
  EXPECT_EQ("2015", Format(civil_year(2015)));
}

TEST(CivilTime, FieldsConstructionLimits) {
  const int kIntMax = std::numeric_limits<int>::max();
  EXPECT_EQ("2038-01-19T03:14:07",
            Format(civil_second(1970, 1, 1, 0, 0, kIntMax)));
  EXPECT_EQ("6121-02-11T05:21:07",
            Format(civil_second(1970, 1, 1, 0, kIntMax, kIntMax)));
  EXPECT_EQ("251104-11-20T12:21:07",
            Format(civil_second(1970, 1, 1, kIntMax, kIntMax, kIntMax)));
  EXPECT_EQ("6130715-05-30T12:21:07",
            Format(civil_second(1970, 1, kIntMax, kIntMax, kIntMax, kIntMax)));
  EXPECT_EQ(
      "185087685-11-26T12:21:07",
      Format(civil_second(1970, kIntMax, kIntMax, kIntMax, kIntMax, kIntMax)));

  const int kIntMin = std::numeric_limits<int>::min();
  EXPECT_EQ("1901-12-13T20:45:52",
            Format(civil_second(1970, 1, 1, 0, 0, kIntMin)));
  EXPECT_EQ("-2182-11-20T18:37:52",
            Format(civil_second(1970, 1, 1, 0, kIntMin, kIntMin)));
  EXPECT_EQ("-247165-02-11T10:37:52",
            Format(civil_second(1970, 1, 1, kIntMin, kIntMin, kIntMin)));
  EXPECT_EQ("-6126776-08-01T10:37:52",
            Format(civil_second(1970, 1, kIntMin, kIntMin, kIntMin, kIntMin)));
  EXPECT_EQ(
      "-185083747-10-31T10:37:52",
      Format(civil_second(1970, kIntMin, kIntMin, kIntMin, kIntMin, kIntMin)));
}

TEST(CivilTime, ExplicitCrossAlignment) {
  //
  // Assign from smaller units -> larger units
  //

  civil_second second(2015, 1, 2, 3, 4, 5);
  EXPECT_EQ("2015-01-02T03:04:05", Format(second));

  civil_minute minute(second);
  EXPECT_EQ("2015-01-02T03:04", Format(minute));

  civil_hour hour(minute);
  EXPECT_EQ("2015-01-02T03", Format(hour));

  civil_day day(hour);
  EXPECT_EQ("2015-01-02", Format(day));

  civil_month month(day);
  EXPECT_EQ("2015-01", Format(month));

  civil_year year(month);
  EXPECT_EQ("2015", Format(year));

  //
  // Now assign from larger units -> smaller units
  //

  month = civil_month(year);
  EXPECT_EQ("2015-01", Format(month));

  day = civil_day(month);
  EXPECT_EQ("2015-01-01", Format(day));

  hour = civil_hour(day);
  EXPECT_EQ("2015-01-01T00", Format(hour));

  minute = civil_minute(hour);
  EXPECT_EQ("2015-01-01T00:00", Format(minute));

  second = civil_second(minute);
  EXPECT_EQ("2015-01-01T00:00:00", Format(second));
}

TEST(CivilTime, ValueSemantics) {
  const civil_hour a(2015, 1, 2, 3);
  const civil_hour b = a;
  const civil_hour c(b);
  civil_hour d;
  d = c;
  EXPECT_EQ("2015-01-02T03", Format(d));
}

TEST(CivilTime, Relational) {
  // Tests that the alignment unit is ignored in comparison.
  const civil_year year(2014);
  const civil_month month(year);
  EXPECT_EQ(year, month);

#define TEST_RELATIONAL(OLDER, YOUNGER) \
  do {                                  \
    EXPECT_EQ(OLDER, OLDER);            \
    EXPECT_NE(OLDER, YOUNGER);          \
    EXPECT_LT(OLDER, YOUNGER);          \
    EXPECT_LE(OLDER, YOUNGER);          \
    EXPECT_GT(YOUNGER, OLDER);          \
    EXPECT_GE(YOUNGER, OLDER);          \
  } while (0)

  // Alignment is ignored in comparison (verified above), so kSecond is used
  // to test comparison in all field positions.
  TEST_RELATIONAL(civil_second(2014, 1, 1, 0, 0, 0),
                  civil_second(2015, 1, 1, 0, 0, 0));
  TEST_RELATIONAL(civil_second(2014, 1, 1, 0, 0, 0),
                  civil_second(2014, 2, 1, 0, 0, 0));
  TEST_RELATIONAL(civil_second(2014, 1, 1, 0, 0, 0),
                  civil_second(2014, 1, 2, 0, 0, 0));
  TEST_RELATIONAL(civil_second(2014, 1, 1, 0, 0, 0),
                  civil_second(2014, 1, 1, 1, 0, 0));
  TEST_RELATIONAL(civil_second(2014, 1, 1, 1, 0, 0),
                  civil_second(2014, 1, 1, 1, 1, 0));
  TEST_RELATIONAL(civil_second(2014, 1, 1, 1, 1, 0),
                  civil_second(2014, 1, 1, 1, 1, 1));

  // Tests the relational operators of two different CivilTime types.
  TEST_RELATIONAL(civil_day(2014, 1, 1), civil_minute(2014, 1, 1, 1, 1));
  TEST_RELATIONAL(civil_day(2014, 1, 1), civil_month(2014, 2));

#undef TEST_RELATIONAL
}

TEST(CivilTime, Arithmetic) {
  civil_second second(2015, 1, 2, 3, 4, 5);
  EXPECT_EQ("2015-01-02T03:04:06", Format(second += 1));
  EXPECT_EQ("2015-01-02T03:04:07", Format(second + 1));
  EXPECT_EQ("2015-01-02T03:04:08", Format(2 + second));
  EXPECT_EQ("2015-01-02T03:04:05", Format(second - 1));
  EXPECT_EQ("2015-01-02T03:04:05", Format(second -= 1));
  EXPECT_EQ("2015-01-02T03:04:05", Format(second++));
  EXPECT_EQ("2015-01-02T03:04:07", Format(++second));
  EXPECT_EQ("2015-01-02T03:04:07", Format(second--));
  EXPECT_EQ("2015-01-02T03:04:05", Format(--second));

  civil_minute minute(2015, 1, 2, 3, 4);
  EXPECT_EQ("2015-01-02T03:05", Format(minute += 1));
  EXPECT_EQ("2015-01-02T03:06", Format(minute + 1));
  EXPECT_EQ("2015-01-02T03:07", Format(2 + minute));
  EXPECT_EQ("2015-01-02T03:04", Format(minute - 1));
  EXPECT_EQ("2015-01-02T03:04", Format(minute -= 1));
  EXPECT_EQ("2015-01-02T03:04", Format(minute++));
  EXPECT_EQ("2015-01-02T03:06", Format(++minute));
  EXPECT_EQ("2015-01-02T03:06", Format(minute--));
  EXPECT_EQ("2015-01-02T03:04", Format(--minute));

  civil_hour hour(2015, 1, 2, 3);
  EXPECT_EQ("2015-01-02T04", Format(hour += 1));
  EXPECT_EQ("2015-01-02T05", Format(hour + 1));
  EXPECT_EQ("2015-01-02T06", Format(2 + hour));
  EXPECT_EQ("2015-01-02T03", Format(hour - 1));
  EXPECT_EQ("2015-01-02T03", Format(hour -= 1));
  EXPECT_EQ("2015-01-02T03", Format(hour++));
  EXPECT_EQ("2015-01-02T05", Format(++hour));
  EXPECT_EQ("2015-01-02T05", Format(hour--));
  EXPECT_EQ("2015-01-02T03", Format(--hour));

  civil_day day(2015, 1, 2);
  EXPECT_EQ("2015-01-03", Format(day += 1));
  EXPECT_EQ("2015-01-04", Format(day + 1));
  EXPECT_EQ("2015-01-05", Format(2 + day));
  EXPECT_EQ("2015-01-02", Format(day - 1));
  EXPECT_EQ("2015-01-02", Format(day -= 1));
  EXPECT_EQ("2015-01-02", Format(day++));
  EXPECT_EQ("2015-01-04", Format(++day));
  EXPECT_EQ("2015-01-04", Format(day--));
  EXPECT_EQ("2015-01-02", Format(--day));

  civil_month month(2015, 1);
  EXPECT_EQ("2015-02", Format(month += 1));
  EXPECT_EQ("2015-03", Format(month + 1));
  EXPECT_EQ("2015-04", Format(2 + month));
  EXPECT_EQ("2015-01", Format(month - 1));
  EXPECT_EQ("2015-01", Format(month -= 1));
  EXPECT_EQ("2015-01", Format(month++));
  EXPECT_EQ("2015-03", Format(++month));
  EXPECT_EQ("2015-03", Format(month--));
  EXPECT_EQ("2015-01", Format(--month));

  civil_year year(2015);
  EXPECT_EQ("2016", Format(year += 1));
  EXPECT_EQ("2017", Format(year + 1));
  EXPECT_EQ("2018", Format(2 + year));
  EXPECT_EQ("2015", Format(year - 1));
  EXPECT_EQ("2015", Format(year -= 1));
  EXPECT_EQ("2015", Format(year++));
  EXPECT_EQ("2017", Format(++year));
  EXPECT_EQ("2017", Format(year--));
  EXPECT_EQ("2015", Format(--year));
}

TEST(CivilTime, ArithmeticLimits) {
  constexpr int kIntMax = std::numeric_limits<int>::max();
  constexpr int kIntMin = std::numeric_limits<int>::min();

  civil_second second(1970, 1, 1, 0, 0, 0);
  second += kIntMax;
  EXPECT_EQ("2038-01-19T03:14:07", Format(second));
  second -= kIntMax;
  EXPECT_EQ("1970-01-01T00:00:00", Format(second));
  second += kIntMin;
  EXPECT_EQ("1901-12-13T20:45:52", Format(second));
  second -= kIntMin;
  EXPECT_EQ("1970-01-01T00:00:00", Format(second));

  civil_minute minute(1970, 1, 1, 0, 0);
  minute += kIntMax;
  EXPECT_EQ("6053-01-23T02:07", Format(minute));
  minute -= kIntMax;
  EXPECT_EQ("1970-01-01T00:00", Format(minute));
  minute += kIntMin;
  EXPECT_EQ("-2114-12-08T21:52", Format(minute));
  minute -= kIntMin;
  EXPECT_EQ("1970-01-01T00:00", Format(minute));

  civil_hour hour(1970, 1, 1, 0);
  hour += kIntMax;
  EXPECT_EQ("246953-10-09T07", Format(hour));
  hour -= kIntMax;
  EXPECT_EQ("1970-01-01T00", Format(hour));
  hour += kIntMin;
  EXPECT_EQ("-243014-03-24T16", Format(hour));
  hour -= kIntMin;
  EXPECT_EQ("1970-01-01T00", Format(hour));

  civil_day day(1970, 1, 1);
  day += kIntMax;
  EXPECT_EQ("5881580-07-11", Format(day));
  day -= kIntMax;
  EXPECT_EQ("1970-01-01", Format(day));
  day += kIntMin;
  EXPECT_EQ("-5877641-06-23", Format(day));
  day -= kIntMin;
  EXPECT_EQ("1970-01-01", Format(day));

  civil_month month(1970, 1);
  month += kIntMax;
  EXPECT_EQ("178958940-08", Format(month));
  month -= kIntMax;
  EXPECT_EQ("1970-01", Format(month));
  month += kIntMin;
  EXPECT_EQ("-178955001-05", Format(month));
  month -= kIntMin;
  EXPECT_EQ("1970-01", Format(month));

  civil_year year(0);
  year += kIntMax;
  EXPECT_EQ("2147483647", Format(year));
  year -= kIntMax;
  EXPECT_EQ("0", Format(year));
  year += kIntMin;
  EXPECT_EQ("-2147483648", Format(year));
  year -= kIntMin;
  EXPECT_EQ("0", Format(year));
}

TEST(CivilTime, Difference) {
  civil_second second(2015, 1, 2, 3, 4, 5);
  EXPECT_EQ(0, second - second);
  EXPECT_EQ(10, (second + 10) - second);
  EXPECT_EQ(-10, (second - 10) - second);

  civil_minute minute(2015, 1, 2, 3, 4);
  EXPECT_EQ(0, minute - minute);
  EXPECT_EQ(10, (minute + 10) - minute);
  EXPECT_EQ(-10, (minute - 10) - minute);

  civil_hour hour(2015, 1, 2, 3);
  EXPECT_EQ(0, hour - hour);
  EXPECT_EQ(10, (hour + 10) - hour);
  EXPECT_EQ(-10, (hour - 10) - hour);

  civil_day day(2015, 1, 2);
  EXPECT_EQ(0, day - day);
  EXPECT_EQ(10, (day + 10) - day);
  EXPECT_EQ(-10, (day - 10) - day);

  civil_month month(2015, 1);
  EXPECT_EQ(0, month - month);
  EXPECT_EQ(10, (month + 10) - month);
  EXPECT_EQ(-10, (month - 10) - month);

  civil_year year(2015);
  EXPECT_EQ(0, year - year);
  EXPECT_EQ(10, (year + 10) - year);
  EXPECT_EQ(-10, (year - 10) - year);
}

TEST(CivilTime, DifferenceLimits) {
  const int kIntMax = std::numeric_limits<int>::max();
  const int kIntMin = std::numeric_limits<int>::min();

  // Check day arithmetic at the end of the year range.
  const civil_day max_day(kIntMax, 12, 31);
  EXPECT_EQ(1, max_day - (max_day - 1));
  EXPECT_EQ(-1, (max_day - 1) - max_day);

  // Check day arithmetic at the end of the year range.
  const civil_day min_day(kIntMin, 1, 1);
  EXPECT_EQ(1, (min_day + 1) - min_day);
  EXPECT_EQ(-1, min_day - (min_day + 1));

  // Check the limits of the return value.
  const civil_day d1(1970, 1, 1);
  const civil_day d2(5881580, 7, 11);
  EXPECT_EQ(kIntMax, d2 - d1);
  EXPECT_EQ(kIntMin, d1 - (d2 + 1));
}

TEST(CivilTime, Properties) {
  civil_second ss(2015, 2, 3, 4, 5, 6);
  EXPECT_EQ(2015, ss.year());
  EXPECT_EQ(2, ss.month());
  EXPECT_EQ(3, ss.day());
  EXPECT_EQ(4, ss.hour());
  EXPECT_EQ(5, ss.minute());
  EXPECT_EQ(6, ss.second());

  civil_minute mm(2015, 2, 3, 4, 5, 6);
  EXPECT_EQ(2015, mm.year());
  EXPECT_EQ(2, mm.month());
  EXPECT_EQ(3, mm.day());
  EXPECT_EQ(4, mm.hour());
  EXPECT_EQ(5, mm.minute());
  EXPECT_EQ(0, mm.second());

  civil_hour hh(2015, 2, 3, 4, 5, 6);
  EXPECT_EQ(2015, hh.year());
  EXPECT_EQ(2, hh.month());
  EXPECT_EQ(3, hh.day());
  EXPECT_EQ(4, hh.hour());
  EXPECT_EQ(0, hh.minute());
  EXPECT_EQ(0, hh.second());

  civil_day d(2015, 2, 3, 4, 5, 6);
  EXPECT_EQ(2015, d.year());
  EXPECT_EQ(2, d.month());
  EXPECT_EQ(3, d.day());
  EXPECT_EQ(0, d.hour());
  EXPECT_EQ(0, d.minute());
  EXPECT_EQ(0, d.second());
  EXPECT_EQ(weekday::tuesday, get_weekday(d));

  civil_month m(2015, 2, 3, 4, 5, 6);
  EXPECT_EQ(2015, m.year());
  EXPECT_EQ(2, m.month());
  EXPECT_EQ(1, m.day());
  EXPECT_EQ(0, m.hour());
  EXPECT_EQ(0, m.minute());
  EXPECT_EQ(0, m.second());

  civil_year y(2015, 2, 3, 4, 5, 6);
  EXPECT_EQ(2015, y.year());
  EXPECT_EQ(1, y.month());
  EXPECT_EQ(1, y.day());
  EXPECT_EQ(0, y.hour());
  EXPECT_EQ(0, y.minute());
  EXPECT_EQ(0, y.second());
}

TEST(CivilTime, NextPrevWeekday) {
  // Jan 1, 1970 was a Thursday.
  const civil_day thursday(1970, 1, 1);
  EXPECT_EQ(weekday::thursday, get_weekday(thursday)) << Format(thursday);

  // Thursday -> Thursday
  civil_day d = next_weekday(thursday, weekday::thursday);
  EXPECT_EQ(7, d - thursday) << Format(d);
  EXPECT_EQ(d - 14, prev_weekday(thursday, weekday::thursday));

  // Thursday -> Friday
  d = next_weekday(thursday, weekday::friday);
  EXPECT_EQ(1, d - thursday) << Format(d);
  EXPECT_EQ(d - 7, prev_weekday(thursday, weekday::friday));

  // Thursday -> Saturday
  d = next_weekday(thursday, weekday::saturday);
  EXPECT_EQ(2, d - thursday) << Format(d);
  EXPECT_EQ(d - 7, prev_weekday(thursday, weekday::saturday));

  // Thursday -> Sunday
  d = next_weekday(thursday, weekday::sunday);
  EXPECT_EQ(3, d - thursday) << Format(d);
  EXPECT_EQ(d - 7, prev_weekday(thursday, weekday::sunday));

  // Thursday -> Monday
  d = next_weekday(thursday, weekday::monday);
  EXPECT_EQ(4, d - thursday) << Format(d);
  EXPECT_EQ(d - 7, prev_weekday(thursday, weekday::monday));

  // Thursday -> Tuesday
  d = next_weekday(thursday, weekday::tuesday);
  EXPECT_EQ(5, d - thursday) << Format(d);
  EXPECT_EQ(d - 7, prev_weekday(thursday, weekday::tuesday));

  // Thursday -> Wednesday
  d = next_weekday(thursday, weekday::wednesday);
  EXPECT_EQ(6, d - thursday) << Format(d);
  EXPECT_EQ(d - 7, prev_weekday(thursday, weekday::wednesday));
}

// NOTE: Run this with --copt=-ftrapv to detect overflow problems.
TEST(CivilTime, DifferenceWithHugeYear) {
  civil_day d1(5881579, 1, 1);
  civil_day d2(5881579, 12, 31);
  EXPECT_EQ(364, d2 - d1);

  d1 = civil_day(-5877640, 1, 1);
  d2 = civil_day(-5877640, 12, 31);
  EXPECT_EQ(365, d2 - d1);

  // Check the limits of the return value with large positive year.
  d1 = civil_day(5881580, 7, 11);
  d2 = civil_day(1970, 1, 1);
  EXPECT_EQ(2147483647, d1 - d2);
  d2 = d2 - 1;
  EXPECT_EQ(-2147483647 - 1, d2 - d1);

  // Check the limits of the return value with large negative year.
  d1 = civil_day(-5877641, 6, 23);
  d2 = civil_day(1969, 12, 31);
  EXPECT_EQ(2147483647, d2 - d1);
  d2 = d2 + 1;
  EXPECT_EQ(-2147483647 - 1, d1 - d2);

  // Check the limits of the return value from either side of year 0.
  d1 = civil_day(-2939806, 9, 26);
  d2 = civil_day(2939805, 4, 6);
  EXPECT_EQ(2147483647, d2 - d1);
  d2 = d2 + 1;
  EXPECT_EQ(-2147483647 - 1, d1 - d2);
}

TEST(CivilTime, NormalizeWithHugeYear) {
  civil_month c(2147483647, 1);
  EXPECT_EQ("2147483647-01", Format(c));
  c = c - 1;  // Causes normalization
  EXPECT_EQ("2147483646-12", Format(c));

  c = civil_month(-2147483647 - 1, 1);
  EXPECT_EQ("-2147483648-01", Format(c));
  c = c + 12;  // Causes normalization
  EXPECT_EQ("-2147483647-01", Format(c));
}

TEST(CivilTime, LeapYears) {
  // Test data for leap years.
  const struct {
    int year;
    int days;
    struct {
      int month;
      int day;
    } leap_day;  // The date of the day after Feb 28.
  } kLeapYearTable[]{
      {1900, 365, {3, 1}},
      {1999, 365, {3, 1}},
      {2000, 366, {2, 29}},  // leap year
      {2001, 365, {3, 1}},
      {2002, 365, {3, 1}},
      {2003, 365, {3, 1}},
      {2004, 366, {2, 29}},  // leap year
      {2005, 365, {3, 1}},
      {2006, 365, {3, 1}},
      {2007, 365, {3, 1}},
      {2008, 366, {2, 29}},  // leap year
      {2009, 365, {3, 1}},
      {2100, 365, {3, 1}},
  };

  for (size_t i = 0; i < (sizeof kLeapYearTable) / (sizeof kLeapYearTable[0]);
       ++i) {
    const int y = kLeapYearTable[i].year;
    const int m = kLeapYearTable[i].leap_day.month;
    const int d = kLeapYearTable[i].leap_day.day;
    const int n = kLeapYearTable[i].days;

    // Tests incrementing through the leap day.
    const civil_day feb28(y, 2, 28);
    const civil_day next_day = feb28 + 1;
    EXPECT_EQ(m, next_day.month());
    EXPECT_EQ(d, next_day.day());

    // Tests difference in days of leap years.
    const civil_year year(feb28);
    const civil_year next_year = year + 1;
    EXPECT_EQ(n, civil_day(next_year) - civil_day(year));
  }
}

TEST(CivilTime, FirstThursdayInMonth) {
  const civil_day nov1(2014, 11, 1);
  const civil_day thursday = prev_weekday(nov1, weekday::thursday) + 7;
  EXPECT_EQ("2014-11-06", Format(thursday));

  // Bonus: Date of Thanksgiving in the United States
  // Rule: Fourth Thursday of November
  const civil_day thanksgiving = thursday + 7 * 3;
  EXPECT_EQ("2014-11-27", Format(thanksgiving));
}

}  // namespace cctz