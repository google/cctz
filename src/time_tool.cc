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

// A command-line tool for exercising the CCTZ library.

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "cctz/civil_time.h"
#include "cctz/time_zone.h"

// Pulls in the aliases from cctz for brevity.
template <typename D>
using time_point = cctz::time_point<D>;
using seconds = cctz::seconds;

// parse() specifiers for command-line time arguments.
const char* const kFormats[] = {
  "%Y   %m   %d   %H   %M   %E*S",
  "%Y - %m - %d %ET %H : %M : %E*S",
  "%Y - %m - %d %H : %M : %E*S",
  "%Y - %m - %d %ET %H : %M",
  "%Y - %m - %d %H : %M",
  "%Y - %m - %d",
  "%a %b %d %H : %M : %E*S %Z %Y",
  "%a %e %b %Y %H : %M : %E*S",
  "%a %b %e %Y %H : %M : %E*S",
  "%e %b %Y %H : %M : %E*S",
  "%b %e %Y %H : %M : %E*S",
  "%a %e %b %Y %H : %M",
  "%a %b %e %Y %H : %M",
  "%e %b %Y %H : %M",
  "%b %e %Y %H : %M",
  "%a %e %b %Y",
  "%a %b %e %Y",
  "%e %b %Y",
  "%b %e %Y",
  nullptr
};

bool ParseTimeSpec(const std::string& args, time_point<seconds>* when) {
  const cctz::time_zone ignored{};
  for (const char* const* fmt = kFormats; *fmt != nullptr; ++fmt) {
    const std::string format = std::string(*fmt) + " %E*z";
    time_point<seconds> tp;
    if (cctz::parse(format, args, ignored, &tp)) {
      *when = tp;
      return true;
    }
  }
  return false;
}

bool ParseCivilSpec(const std::string& args, cctz::civil_second* when) {
  const cctz::time_zone utc = cctz::utc_time_zone();
  for (const char* const* fmt = kFormats; *fmt != nullptr; ++fmt) {
    time_point<seconds> tp;
    if (cctz::parse(*fmt, args, utc, &tp)) {
      *when = cctz::convert(tp, utc);
      return true;
    }
  }
  return false;
}

const char* WeekDayName(cctz::weekday wd) {
  switch (wd) {
    case cctz::weekday::monday: return "Mon";
    case cctz::weekday::tuesday: return "Tue";
    case cctz::weekday::wednesday: return "Wed";
    case cctz::weekday::thursday: return "Thu";
    case cctz::weekday::friday: return "Fri";
    case cctz::weekday::saturday: return "Sat";
    case cctz::weekday::sunday: return "Sun";
  }
  return "XXX";
}

std::string FormatTimeInZone(const std::string& fmt, time_point<seconds> when,
                             cctz::time_zone zone) {
  std::ostringstream oss;
  oss << std::setw(36) << std::left << cctz::format(fmt, when, zone);
  cctz::time_zone::absolute_lookup al = zone.lookup(when);
  oss << " [wd=" << WeekDayName(cctz::get_weekday(al.cs))
      << " yd=" << std::setw(3) << std::setfill('0')
      << std::right << cctz::get_yearday(al.cs)
      << " dst=" << (al.is_dst ? 'T' : 'F')
      << " off=" << std::showpos << al.offset << std::noshowpos << "]";
  return oss.str();
}

void ZoneInfo(const std::string& label, cctz::time_zone tz) {
  std::string version = tz.version();
  if (version.empty()) version = "<unknown>";
  std::cout << label << tz.name() << " [ver=" << version << " "
            << tz.description() << "]\n";
}

void InstantInfo(const std::string& label, const std::string& fmt,
                 time_point<seconds> when, cctz::time_zone zone) {
  const cctz::time_zone loc = cctz::local_time_zone();  // might == zone
  const cctz::time_zone utc = cctz::utc_time_zone();  // might == zone
  const std::string time_label = "time_t";
  const std::string utc_label = "UTC";
  const std::string loc_label = "local";
  const std::string zone_label = "in-tz";  // perhaps zone.name()?
  int width = 2 + static_cast<int>(
                      std::max(std::max(time_label.size(), utc_label.size()),
                               std::max(loc_label.size(), zone_label.size())));
  std::cout << label << " {\n";
  std::cout << std::setw(width) << std::right << time_label << ": ";
  std::cout << std::setw(10) << cctz::format("%s", when, utc);
  std::cout << "\n";
  std::cout << std::setw(width) << std::right << utc_label << ": ";
  std::cout << FormatTimeInZone(fmt, when, utc) << "\n";
  std::cout << std::setw(width) << std::right << loc_label << ": ";
  std::cout << FormatTimeInZone(fmt, when, loc) << "\n";
  std::cout << std::setw(width) << std::right << zone_label << ": ";
  std::cout << FormatTimeInZone(fmt, when, zone) << "\n";
  std::cout << "}\n";
}

// Report everything we know about a cctz::civil_second (YMDHMS).
void CivilInfo(const std::string& fmt, const cctz::civil_second cs,
               cctz::time_zone zone) {
  ZoneInfo("tz: ", zone);
  cctz::time_zone::civil_lookup cl = zone.lookup(cs);
  switch (cl.kind) {
    case cctz::time_zone::civil_lookup::UNIQUE: {
      std::cout << "kind: UNIQUE\n";
      InstantInfo("when", fmt, cl.pre, zone);
      break;
    }
    case cctz::time_zone::civil_lookup::SKIPPED: {
      std::cout << "kind: SKIPPED\n";
      InstantInfo("post", fmt, cl.post, zone);  // might == trans-1
      InstantInfo("trans-1", fmt, cl.trans - seconds(1), zone);
      InstantInfo("trans", fmt, cl.trans, zone);
      InstantInfo("pre", fmt, cl.pre, zone);  // might == trans
      break;
    }
    case cctz::time_zone::civil_lookup::REPEATED: {
      std::cout << "kind: REPEATED\n";
      InstantInfo("pre", fmt, cl.pre, zone);  // might == trans-1
      InstantInfo("trans-1", fmt, cl.trans - seconds(1), zone);
      InstantInfo("trans", fmt, cl.trans, zone);
      InstantInfo("post", fmt, cl.post, zone);  // might == trans
      break;
    }
  }
}

// Report everything we know about a time_point<seconds>.
void TimeInfo(const std::string& fmt, time_point<seconds> when,
              cctz::time_zone zone) {
  ZoneInfo("tz: ", zone);
  std::cout << "kind: UNIQUE\n";
  InstantInfo("when", fmt, when, zone);
}

// Report everything we know about a time_zone.
void ZoneDump(bool zdump, const std::string& fmt, cctz::time_zone zone,
              cctz::year_t lo_year, cctz::year_t hi_year) {
  const cctz::time_zone utc = cctz::utc_time_zone();
  if (zdump) {
    std::cout << zone.name() << "  "
              << std::numeric_limits<seconds::rep>::min()
              << " = NULL\n";
    std::cout << zone.name() << "  "
              << std::numeric_limits<seconds::rep>::min() + 86400
              << " = NULL\n";
  } else {
    ZoneInfo("", zone);
  }

  auto tp = cctz::convert(cctz::civil_second(lo_year, 1, 1, 0, 0, -1), zone);
  cctz::time_zone::civil_transition trans;
  while (zone.next_transition(tp, &trans)) {
    if (trans.from.year() >= hi_year && trans.to.year() >= hi_year) break;
    tp = zone.lookup(trans.to).trans;
    if (!zdump) std::cout << "\n";
    for (int count_down = 1; count_down >= 0; --count_down) {
      auto ttp = tp - seconds(count_down);
      if (zdump) {
        std::cout << zone.name() << "  " << cctz::format("%c UT", ttp, utc)
                  << " = " << cctz::format("%c %Z", ttp, zone);
      } else {
        std::cout << std::setw(10) << std::chrono::system_clock::to_time_t(ttp);
        std::cout << " = " << cctz::format(fmt, ttp, utc);
        std::cout << " = " << cctz::format(fmt, ttp, zone);
      }
      auto al = zone.lookup(ttp);
      if (zdump) {
        std::cout << " isdst=" << (al.is_dst ? '1' : '0')
                  << " gmtoff=" << al.offset << "\n";
      } else {
        const char* wd = WeekDayName(get_weekday(al.cs));
        std::cout << " [wd=" << wd << " dst=" << (al.is_dst ? 'T' : 'F')
                  << " off=" << al.offset << "]\n";
      }
    }
  }

  if (zdump) {
    std::cout << zone.name() << "  "
              << std::numeric_limits<seconds::rep>::max() - 86400
              << " = NULL\n";
    std::cout << zone.name() << "  "
              << std::numeric_limits<seconds::rep>::max()
              << " = NULL\n";
  }
}

const char* Basename(const char* p) {
  if (const char* b = std::strrchr(p, '/')) return ++b;
  return p;
}

// std::regex doesn't work before gcc 4.9.
bool LooksLikeNegOffset(const char* s) {
  if (s[0] == '-' && std::isdigit(s[1]) && std::isdigit(s[2])) {
    int i = (s[3] == ':') ? 4 : 3;
    if (std::isdigit(s[i]) && std::isdigit(s[i + 1])) {
      return s[i + 2] == '\0';
    }
  }
  return false;
}

std::vector<std::string> StrSplit(char sep, const std::string& s) {
  std::vector<std::string> v;
  // An empty string value corresponds to an empty vector, not a vector
  // with a single, empty string.
  if (!s.empty()) {
    std::string::size_type pos = 0;
    for (;;) {
      std::string::size_type spos = s.find(sep, pos);
      if (spos == std::string::npos) break;
      v.push_back(s.substr(pos, spos - pos));
      pos = spos + 1;
    }
    v.push_back(s.substr(pos));
  }
  return v;
}

// Parses [<lo-year>,]<hi-year>.
bool ParseYearRange(bool zdump, const std::string& args,
                    cctz::year_t* lo_year, cctz::year_t* hi_year) {
  std::size_t pos = 0;
  std::size_t digit_pos = pos + (args[pos] == '-' ? 1 : 0);
  if (digit_pos >= args.size() || !std::isdigit(args[digit_pos])) {
    return false;
  }
  const cctz::year_t first = std::stoll(args, &pos);
  if (pos == args.size()) {
    *lo_year = (zdump ? -292277022656 : first);
    *hi_year = (zdump ? first : first + 1);
    return true;
  }
  if (args[pos] != ' ' || ++pos == args.size()) {
    // Any comma was already converted to a space.
    return false;
  }
  digit_pos = pos + (args[pos] == '-' ? 1 : 0);
  if (digit_pos >= args.size() || !std::isdigit(args[digit_pos])) {
    return false;
  }
  const std::string rem = args.substr(pos);
  const cctz::year_t second = std::stoll(rem, &pos);
  if (pos == rem.size()) {
    *lo_year = first;
    *hi_year = (zdump ? second : second + 1);
    return true;
  }
  return false;
}

int main(int argc, const char** argv) {
  const char* argv0 = (argc > 0) ? (argc--, *argv++) : (argc = 0, "time_tool");
  const std::string prog = Basename(argv0);

  // Escape arguments that look like negative offsets so that they
  // don't look like flags.
  std::vector<std::string> eargs;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--") == 0) break;
    if (LooksLikeNegOffset(argv[i])) {
      eargs.push_back(" ");  // space will later be ignorned
      eargs.back().append(argv[i]);
      argv[i] = eargs.back().c_str();
    }
  }

  // Determine the time zone and other options.
  std::string zones = "localtime";
  std::string fmt = "%Y-%m-%d %H:%M:%S %E*z (%Z)";
  bool zone_dump = (prog == "zone_dump");
  bool zdump = false;  // Use zdump(8) format.
  int optind = 0;
  int opterr = 0;
  for (; optind < argc && opterr == 0; ++optind) {
    const char* opt = argv[optind];
    if (*opt++ != '-') break;
    if (*opt != '-') {  // short options
      while (char c = *opt++) {
        if (c == 'z') {
          if (*opt != '\0') {
            zones = opt;
            break;
          }
          if (optind + 1 == argc) {
            std::cerr << argv0 << ": option requires an argument -- 'z'\n";
            ++opterr;
            break;
          }
          zones = argv[++optind];
        } else if (c == 'f') {
          if (*opt != '\0') {
            fmt = opt;
            break;
          }
          if (optind + 1 == argc) {
            std::cerr << argv0 << ": option requires an argument -- 'f'\n";
            ++opterr;
            break;
          }
          fmt = argv[++optind];
        } else if (c == 'D') {
          zdump = true;
        } else if (c == 'd') {
          zone_dump = true;
        } else {
          std::cerr << argv0 << ": invalid option -- '" << c << "'\n";
          ++opterr;
          break;
        }
      }
    } else {  // long options
      if (*++opt == '\0') {  // "--"
        ++optind;
        break;
      }
      if (std::strcmp(opt, "tz") == 0) {
        if (optind + 1 == argc) {
          std::cerr << argv0 << ": option '--tz' requires an argument\n";
          ++opterr;
        } else {
          zones = argv[++optind];
        }
      } else if (std::strncmp(opt, "tz=", 3) == 0) {
        zones = opt + 3;
      } else if (std::strcmp(opt, "fmt") == 0) {
        if (optind + 1 == argc) {
          std::cerr << argv0 << ": option '--fmt' requires an argument\n";
          ++opterr;
        } else {
          fmt = argv[++optind];
        }
      } else if (std::strncmp(opt, "fmt=", 4) == 0) {
        fmt = opt + 4;
      } else if (std::strcmp(opt, "zdump") == 0) {
        zdump = true;
      } else if (std::strcmp(opt, "zone_dump") == 0) {
        zone_dump = true;
      } else {
        std::cerr << argv0 << ": unrecognized option '--" << opt << "'\n";
        ++opterr;
      }
    }
  }
  if (opterr != 0) {
    std::cerr << "Usage: " << prog << " [--tz=<zone>[,...]] [--fmt=<fmt>]";
    if (prog == "zone_dump") {
      std::cerr << " [[<lo-year>,]<hi-year>|<time-spec>]\n";
      std::cerr << "  Default years are last year and next year,"
                << " respectively.\n";
    } else {
      std::cerr << " [<time-spec>]\n";
    }
    std::cerr << "  Default <time-spec> is 'now'.\n";
    return 1;
  }

  std::string args;
  for (int i = optind; i < argc; ++i) {
    if (i != optind) args += " ";
    args += argv[i];
  }
  std::replace(args.begin(), args.end(), ',', ' ');
  std::replace(args.begin(), args.end(), '/', '-');

  // Determine the time point.
  time_point<seconds> tp =
      std::chrono::time_point_cast<seconds>(std::chrono::system_clock::now());
  bool have_time = ParseTimeSpec(args, &tp);
  if (!have_time && !args.empty()) {
    std::string spec = args.substr((args[0] == '@') ? 1 : 0);
    if ((spec.size() > 0 && std::isdigit(spec[0])) ||
        (spec.size() > 1 && spec[0] == '-' && std::isdigit(spec[1]))) {
      std::size_t end;
      const time_t t = std::stoll(spec, &end);
      if (end == spec.size()) {
        tp = std::chrono::time_point_cast<seconds>(
            std::chrono::system_clock::from_time_t(t));
        have_time = true;
      }
    }
  }

  std::string leader = "";
  for (const std::string& tz : StrSplit(',', zones)) {
    std::cout << leader;
    cctz::time_zone zone;
    if (tz == "localtime") {
      zone = cctz::local_time_zone();
    } else if (!cctz::load_time_zone(tz, &zone)) {
      std::cerr << tz << ": Unrecognized time zone\n";
      return 1;
    }

    cctz::civil_second when = cctz::convert(tp, zone);
    bool have_civil = !have_time && ParseCivilSpec(args, &when);

    if (zone_dump || zdump) {
      cctz::year_t lo_year = (zdump ? -292277026596 : when.year());
      cctz::year_t hi_year = (zdump ? 292277026596 : when.year() + 1);
      if (!args.empty() && !ParseYearRange(zdump, args, &lo_year, &hi_year)) {
        if (!have_time && !have_civil) {
          std::cerr << args << ": Malformed year range\n";
          return 1;
        }
      }
      ZoneDump(zdump, fmt, zone, lo_year, hi_year);
      leader = "---\n";
    } else {
      if (!have_civil && !have_time && !args.empty()) {
        std::cerr << args << ": Malformed time spec\n";
        return 1;
      }
      if (have_civil) {
        CivilInfo(fmt, when, zone);
      } else {
        TimeInfo(fmt, tp, zone);
      }
      leader = "\n";
    }
  }
}
