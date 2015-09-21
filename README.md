This is not an official Google product.

# Overview

CCTZ (C++ Time Zone) is a library for translating between absolute times and
civil times (see the "Fundamental Concepts" section below for an explanation of
these terms) using the rules defined by a time zone.

This library currently works on Linux, using the standard IANA time zone data
installed on the system in `/usr/share/zoneinfo`.

CCTZ is built using http://bazel.io and tested using
https://github.com/google/googletest

## Fundamental Concepts

[ The slides presented at [CppCon '15](http://cppcon.org) http://goo.gl/ofof4N ]

There are two ways to represent time: as an *Absolute Time*, and as a *Civil
Time*. An absolute time uniquely and universally represents a specific instant
in time. Every event occurs at a specific absolute time, and everyone in the
world will agree on the absolute time when the event occurred. A `time_t` is a
well-known absolute time type. Consider the moment when Neil Armstrong first put
his left foot on the moon. He did that for the first time only once. He did not
do it again an hour later for the audience one time zone to the west. Everyone
in the world will agree on the *absolute time* when this event happened.

On the other hand, not everyone will agree on the *civil time* when that giant
leap was made. A civil time is represented as _six individual fields_ that
represent a year, month, day, hour, minute, and second. These six fields
represent the time as defined by some local government. Your civil time matches
the values shown on the clock hanging on your wall and the Dilbert calendar on
your desk. Your friend living across the country may, at the same moment, have a
different civil time showing on their Far Side calendar and clock. For example,
if you lived in New York on July 20, 1969 you witnessed Neil Armstrong's small
step at 10:56 in the evening, whereas your friend in San Francisco saw the
same thing at 7:56, and your pen pal in Sydney saw it while eating lunch at
12:56 on July 21. You all would agree on the absolute time of the event, but
you'd disagree about the civil time.

Time zones are geo-political regions within which rules are shared to convert
between absolute times and civil times. The geographical nature of time zones is
evident in their identifiers, which look like "America/New_York",
"America/Los_Angeles", and "Australia/Sydney". A time-zone's rules include
things like the region's offset from the UTC time standard, daylight-saving
adjustments, and short abbreviation strings. Since these rules may change at the
whim of the region's local government, time zones have a history of disparate
rules that apply only for certain periods. Time zones are tremendously
complicated, which is why you should always let a time library do time-zone
calculations for you.

Time zones define the relationship between absolute and civil times. Given an
absolute or civil time and a time zone, you can compute the other, as shown
in the example below.

```
Civil Time = F(Absolute Time, Time Zone)
Absolute Time = F(Civil Time, Time Zone)
```

The concepts described thus far&#8212;absolute time, civil time, and time
zone&#8212;are universal concepts that apply to _all programming languages_
equally because they describe time in the real world. Different programming
languages and libraries may model these concepts differently with different
classes and sometimes even different names, but these fundamental concepts and
relationships will still exist.

## These concepts in CCTZ

An *absolute* time is represented by a `cctz::time_point`.
A *civil* time is represented by a `cctz::Breakdown`, or even separate integers.
A *time zone* is represented by a `cctz::TimeZone`.

XXX: See the cctz.h file for more details.
