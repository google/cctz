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
#include <future>
#include <limits>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

using std::chrono::system_clock;

namespace cctz {

namespace {

// A list of known time-zone names.
const char* const kTimeZoneNames[] = {
  "Africa/Abidjan",
  "Africa/Accra",
  "Africa/Addis_Ababa",
  "Africa/Algiers",
  "Africa/Asmara",
  "Africa/Asmera",
  "Africa/Bamako",
  "Africa/Bangui",
  "Africa/Banjul",
  "Africa/Bissau",
  "Africa/Blantyre",
  "Africa/Brazzaville",
  "Africa/Bujumbura",
  "Africa/Cairo",
  "Africa/Casablanca",
  "Africa/Ceuta",
  "Africa/Conakry",
  "Africa/Dakar",
  "Africa/Dar_es_Salaam",
  "Africa/Djibouti",
  "Africa/Douala",
  "Africa/El_Aaiun",
  "Africa/Freetown",
  "Africa/Gaborone",
  "Africa/Harare",
  "Africa/Johannesburg",
  "Africa/Juba",
  "Africa/Kampala",
  "Africa/Khartoum",
  "Africa/Kigali",
  "Africa/Kinshasa",
  "Africa/Lagos",
  "Africa/Libreville",
  "Africa/Lome",
  "Africa/Luanda",
  "Africa/Lubumbashi",
  "Africa/Lusaka",
  "Africa/Malabo",
  "Africa/Maputo",
  "Africa/Maseru",
  "Africa/Mbabane",
  "Africa/Mogadishu",
  "Africa/Monrovia",
  "Africa/Nairobi",
  "Africa/Ndjamena",
  "Africa/Niamey",
  "Africa/Nouakchott",
  "Africa/Ouagadougou",
  "Africa/Porto-Novo",
  "Africa/Sao_Tome",
  "Africa/Timbuktu",
  "Africa/Tripoli",
  "Africa/Tunis",
  "Africa/Windhoek",
  "America/Adak",
  "America/Anchorage",
  "America/Anguilla",
  "America/Antigua",
  "America/Araguaina",
  "America/Argentina/Buenos_Aires",
  "America/Argentina/Catamarca",
  "America/Argentina/ComodRivadavia",
  "America/Argentina/Cordoba",
  "America/Argentina/Jujuy",
  "America/Argentina/La_Rioja",
  "America/Argentina/Mendoza",
  "America/Argentina/Rio_Gallegos",
  "America/Argentina/Salta",
  "America/Argentina/San_Juan",
  "America/Argentina/San_Luis",
  "America/Argentina/Tucuman",
  "America/Argentina/Ushuaia",
  "America/Aruba",
  "America/Asuncion",
  "America/Atikokan",
  "America/Atka",
  "America/Bahia",
  "America/Bahia_Banderas",
  "America/Barbados",
  "America/Belem",
  "America/Belize",
  "America/Blanc-Sablon",
  "America/Boa_Vista",
  "America/Bogota",
  "America/Boise",
  "America/Buenos_Aires",
  "America/Cambridge_Bay",
  "America/Campo_Grande",
  "America/Cancun",
  "America/Caracas",
  "America/Catamarca",
  "America/Cayenne",
  "America/Cayman",
  "America/Chicago",
  "America/Chihuahua",
  "America/Coral_Harbour",
  "America/Cordoba",
  "America/Costa_Rica",
  "America/Creston",
  "America/Cuiaba",
  "America/Curacao",
  "America/Danmarkshavn",
  "America/Dawson",
  "America/Dawson_Creek",
  "America/Denver",
  "America/Detroit",
  "America/Dominica",
  "America/Edmonton",
  "America/Eirunepe",
  "America/El_Salvador",
  "America/Ensenada",
  "America/Fort_Wayne",
  "America/Fortaleza",
  "America/Glace_Bay",
  "America/Godthab",
  "America/Goose_Bay",
  "America/Grand_Turk",
  "America/Grenada",
  "America/Guadeloupe",
  "America/Guatemala",
  "America/Guayaquil",
  "America/Guyana",
  "America/Halifax",
  "America/Havana",
  "America/Hermosillo",
  "America/Indiana/Indianapolis",
  "America/Indiana/Knox",
  "America/Indiana/Marengo",
  "America/Indiana/Petersburg",
  "America/Indiana/Tell_City",
  "America/Indiana/Vevay",
  "America/Indiana/Vincennes",
  "America/Indiana/Winamac",
  "America/Indianapolis",
  "America/Inuvik",
  "America/Iqaluit",
  "America/Jamaica",
  "America/Jujuy",
  "America/Juneau",
  "America/Kentucky/Louisville",
  "America/Kentucky/Monticello",
  "America/Knox_IN",
  "America/Kralendijk",
  "America/La_Paz",
  "America/Lima",
  "America/Los_Angeles",
  "America/Louisville",
  "America/Lower_Princes",
  "America/Maceio",
  "America/Managua",
  "America/Manaus",
  "America/Marigot",
  "America/Martinique",
  "America/Matamoros",
  "America/Mazatlan",
  "America/Mendoza",
  "America/Menominee",
  "America/Merida",
  "America/Metlakatla",
  "America/Mexico_City",
  "America/Miquelon",
  "America/Moncton",
  "America/Monterrey",
  "America/Montevideo",
  "America/Montreal",
  "America/Montserrat",
  "America/Nassau",
  "America/New_York",
  "America/Nipigon",
  "America/Nome",
  "America/Noronha",
  "America/North_Dakota/Beulah",
  "America/North_Dakota/Center",
  "America/North_Dakota/New_Salem",
  "America/Ojinaga",
  "America/Panama",
  "America/Pangnirtung",
  "America/Paramaribo",
  "America/Phoenix",
  "America/Port-au-Prince",
  "America/Port_of_Spain",
  "America/Porto_Acre",
  "America/Porto_Velho",
  "America/Puerto_Rico",
  "America/Rainy_River",
  "America/Rankin_Inlet",
  "America/Recife",
  "America/Regina",
  "America/Resolute",
  "America/Rio_Branco",
  "America/Rosario",
  "America/Santa_Isabel",
  "America/Santarem",
  "America/Santiago",
  "America/Santo_Domingo",
  "America/Sao_Paulo",
  "America/Scoresbysund",
  "America/Shiprock",
  "America/Sitka",
  "America/St_Barthelemy",
  "America/St_Johns",
  "America/St_Kitts",
  "America/St_Lucia",
  "America/St_Thomas",
  "America/St_Vincent",
  "America/Swift_Current",
  "America/Tegucigalpa",
  "America/Thule",
  "America/Thunder_Bay",
  "America/Tijuana",
  "America/Toronto",
  "America/Tortola",
  "America/Vancouver",
  "America/Virgin",
  "America/Whitehorse",
  "America/Winnipeg",
  "America/Yakutat",
  "America/Yellowknife",
  "Antarctica/Casey",
  "Antarctica/Davis",
  "Antarctica/DumontDUrville",
  "Antarctica/Macquarie",
  "Antarctica/Mawson",
  "Antarctica/McMurdo",
  "Antarctica/Palmer",
  "Antarctica/Rothera",
  "Antarctica/South_Pole",
  "Antarctica/Syowa",
  "Antarctica/Troll",
  "Antarctica/Vostok",
  "Arctic/Longyearbyen",
  "Asia/Aden",
  "Asia/Almaty",
  "Asia/Amman",
  "Asia/Anadyr",
  "Asia/Aqtau",
  "Asia/Aqtobe",
  "Asia/Ashgabat",
  "Asia/Ashkhabad",
  "Asia/Baghdad",
  "Asia/Bahrain",
  "Asia/Baku",
  "Asia/Bangkok",
  "Asia/Beirut",
  "Asia/Bishkek",
  "Asia/Brunei",
  "Asia/Calcutta",
  "Asia/Chita",
  "Asia/Choibalsan",
  "Asia/Chongqing",
  "Asia/Chungking",
  "Asia/Colombo",
  "Asia/Dacca",
  "Asia/Damascus",
  "Asia/Dhaka",
  "Asia/Dili",
  "Asia/Dubai",
  "Asia/Dushanbe",
  "Asia/Gaza",
  "Asia/Harbin",
  "Asia/Hebron",
  "Asia/Ho_Chi_Minh",
  "Asia/Hong_Kong",
  "Asia/Hovd",
  "Asia/Irkutsk",
  "Asia/Istanbul",
  "Asia/Jakarta",
  "Asia/Jayapura",
  "Asia/Jerusalem",
  "Asia/Kabul",
  "Asia/Kamchatka",
  "Asia/Karachi",
  "Asia/Kashgar",
  "Asia/Kathmandu",
  "Asia/Katmandu",
  "Asia/Khandyga",
  "Asia/Kolkata",
  "Asia/Krasnoyarsk",
  "Asia/Kuala_Lumpur",
  "Asia/Kuching",
  "Asia/Kuwait",
  "Asia/Macao",
  "Asia/Macau",
  "Asia/Magadan",
  "Asia/Makassar",
  "Asia/Manila",
  "Asia/Muscat",
  "Asia/Nicosia",
  "Asia/Novokuznetsk",
  "Asia/Novosibirsk",
  "Asia/Omsk",
  "Asia/Oral",
  "Asia/Phnom_Penh",
  "Asia/Pontianak",
  "Asia/Pyongyang",
  "Asia/Qatar",
  "Asia/Qyzylorda",
  "Asia/Rangoon",
  "Asia/Riyadh",
  "Asia/Saigon",
  "Asia/Sakhalin",
  "Asia/Samarkand",
  "Asia/Seoul",
  "Asia/Shanghai",
  "Asia/Singapore",
  "Asia/Srednekolymsk",
  "Asia/Taipei",
  "Asia/Tashkent",
  "Asia/Tbilisi",
  "Asia/Tehran",
  "Asia/Tel_Aviv",
  "Asia/Thimbu",
  "Asia/Thimphu",
  "Asia/Tokyo",
  "Asia/Ujung_Pandang",
  "Asia/Ulaanbaatar",
  "Asia/Ulan_Bator",
  "Asia/Urumqi",
  "Asia/Ust-Nera",
  "Asia/Vientiane",
  "Asia/Vladivostok",
  "Asia/Yakutsk",
  "Asia/Yekaterinburg",
  "Asia/Yerevan",
  "Atlantic/Azores",
  "Atlantic/Bermuda",
  "Atlantic/Canary",
  "Atlantic/Cape_Verde",
  "Atlantic/Faeroe",
  "Atlantic/Faroe",
  "Atlantic/Jan_Mayen",
  "Atlantic/Madeira",
  "Atlantic/Reykjavik",
  "Atlantic/South_Georgia",
  "Atlantic/St_Helena",
  "Atlantic/Stanley",
  "Australia/ACT",
  "Australia/Adelaide",
  "Australia/Brisbane",
  "Australia/Broken_Hill",
  "Australia/Canberra",
  "Australia/Currie",
  "Australia/Darwin",
  "Australia/Eucla",
  "Australia/Hobart",
  "Australia/LHI",
  "Australia/Lindeman",
  "Australia/Lord_Howe",
  "Australia/Melbourne",
  "Australia/NSW",
  "Australia/North",
  "Australia/Perth",
  "Australia/Queensland",
  "Australia/South",
  "Australia/Sydney",
  "Australia/Tasmania",
  "Australia/Victoria",
  "Australia/West",
  "Australia/Yancowinna",
  "Brazil/Acre",
  "Brazil/DeNoronha",
  "Brazil/East",
  "Brazil/West",
  "CET",
  "CST6CDT",
  "Canada/Atlantic",
  "Canada/Central",
  "Canada/East-Saskatchewan",
  "Canada/Eastern",
  "Canada/Mountain",
  "Canada/Newfoundland",
  "Canada/Pacific",
  "Canada/Saskatchewan",
  "Canada/Yukon",
  "Chile/Continental",
  "Chile/EasterIsland",
  "Cuba",
  "EET",
  "EST",
  "EST5EDT",
  "Egypt",
  "Eire",
  "Etc/GMT",
  "Etc/GMT+0",
  "Etc/GMT+1",
  "Etc/GMT+10",
  "Etc/GMT+11",
  "Etc/GMT+12",
  "Etc/GMT+2",
  "Etc/GMT+3",
  "Etc/GMT+4",
  "Etc/GMT+5",
  "Etc/GMT+6",
  "Etc/GMT+7",
  "Etc/GMT+8",
  "Etc/GMT+9",
  "Etc/GMT-0",
  "Etc/GMT-1",
  "Etc/GMT-10",
  "Etc/GMT-11",
  "Etc/GMT-12",
  "Etc/GMT-13",
  "Etc/GMT-14",
  "Etc/GMT-2",
  "Etc/GMT-3",
  "Etc/GMT-4",
  "Etc/GMT-5",
  "Etc/GMT-6",
  "Etc/GMT-7",
  "Etc/GMT-8",
  "Etc/GMT-9",
  "Etc/GMT0",
  "Etc/Greenwich",
  "Etc/UCT",
  "Etc/UTC",
  "Etc/Universal",
  "Etc/Zulu",
  "Europe/Amsterdam",
  "Europe/Andorra",
  "Europe/Athens",
  "Europe/Belfast",
  "Europe/Belgrade",
  "Europe/Berlin",
  "Europe/Bratislava",
  "Europe/Brussels",
  "Europe/Bucharest",
  "Europe/Budapest",
  "Europe/Busingen",
  "Europe/Chisinau",
  "Europe/Copenhagen",
  "Europe/Dublin",
  "Europe/Gibraltar",
  "Europe/Guernsey",
  "Europe/Helsinki",
  "Europe/Isle_of_Man",
  "Europe/Istanbul",
  "Europe/Jersey",
  "Europe/Kaliningrad",
  "Europe/Kiev",
  "Europe/Lisbon",
  "Europe/Ljubljana",
  "Europe/London",
  "Europe/Luxembourg",
  "Europe/Madrid",
  "Europe/Malta",
  "Europe/Mariehamn",
  "Europe/Minsk",
  "Europe/Monaco",
  "Europe/Moscow",
  "Europe/Nicosia",
  "Europe/Oslo",
  "Europe/Paris",
  "Europe/Podgorica",
  "Europe/Prague",
  "Europe/Riga",
  "Europe/Rome",
  "Europe/Samara",
  "Europe/San_Marino",
  "Europe/Sarajevo",
  "Europe/Simferopol",
  "Europe/Skopje",
  "Europe/Sofia",
  "Europe/Stockholm",
  "Europe/Tallinn",
  "Europe/Tirane",
  "Europe/Tiraspol",
  "Europe/Uzhgorod",
  "Europe/Vaduz",
  "Europe/Vatican",
  "Europe/Vienna",
  "Europe/Vilnius",
  "Europe/Volgograd",
  "Europe/Warsaw",
  "Europe/Zagreb",
  "Europe/Zaporozhye",
  "Europe/Zurich",
  "GB",
  "GB-Eire",
  "GMT",
  "GMT+0",
  "GMT-0",
  "GMT0",
  "Greenwich",
  "HST",
  "Hongkong",
  "Iceland",
  "Indian/Antananarivo",
  "Indian/Chagos",
  "Indian/Christmas",
  "Indian/Cocos",
  "Indian/Comoro",
  "Indian/Kerguelen",
  "Indian/Mahe",
  "Indian/Maldives",
  "Indian/Mauritius",
  "Indian/Mayotte",
  "Indian/Reunion",
  "Iran",
  "Israel",
  "Jamaica",
  "Japan",
  "Kwajalein",
  "Libya",
  "MET",
  "MST",
  "MST7MDT",
  "Mexico/BajaNorte",
  "Mexico/BajaSur",
  "Mexico/General",
  "NZ",
  "NZ-CHAT",
  "Navajo",
  "PRC",
  "PST8PDT",
  "Pacific/Apia",
  "Pacific/Auckland",
  "Pacific/Bougainville",
  "Pacific/Chatham",
  "Pacific/Chuuk",
  "Pacific/Easter",
  "Pacific/Efate",
  "Pacific/Enderbury",
  "Pacific/Fakaofo",
  "Pacific/Fiji",
  "Pacific/Funafuti",
  "Pacific/Galapagos",
  "Pacific/Gambier",
  "Pacific/Guadalcanal",
  "Pacific/Guam",
  "Pacific/Honolulu",
  "Pacific/Johnston",
  "Pacific/Kiritimati",
  "Pacific/Kosrae",
  "Pacific/Kwajalein",
  "Pacific/Majuro",
  "Pacific/Marquesas",
  "Pacific/Midway",
  "Pacific/Nauru",
  "Pacific/Niue",
  "Pacific/Norfolk",
  "Pacific/Noumea",
  "Pacific/Pago_Pago",
  "Pacific/Palau",
  "Pacific/Pitcairn",
  "Pacific/Pohnpei",
  "Pacific/Ponape",
  "Pacific/Port_Moresby",
  "Pacific/Rarotonga",
  "Pacific/Saipan",
  "Pacific/Samoa",
  "Pacific/Tahiti",
  "Pacific/Tarawa",
  "Pacific/Tongatapu",
  "Pacific/Truk",
  "Pacific/Wake",
  "Pacific/Wallis",
  "Pacific/Yap",
  "Poland",
  "Portugal",
  "ROC",
  "ROK",
  "Singapore",
  "Turkey",
  "UCT",
  "US/Alaska",
  "US/Aleutian",
  "US/Arizona",
  "US/Central",
  "US/East-Indiana",
  "US/Eastern",
  "US/Hawaii",
  "US/Indiana-Starke",
  "US/Michigan",
  "US/Mountain",
  "US/Pacific",
  "US/Pacific-New",
  "US/Samoa",
  "UTC",
  "Universal",
  "W-SU",
  "WET",
  "Zulu",
  nullptr
};

// Helper to return a loaded time zone by value (UTC on error).
TimeZone LoadZone(const std::string& name) {
  TimeZone tz;
  LoadTimeZone(name, &tz);
  return tz;
}

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

}  // namespace

TEST(TimeZones, LoadZonesConcurrently) {
  std::promise<void> ready_promise;
  std::shared_future<void> ready_future(ready_promise.get_future());
  auto load_zones = [ready_future](std::promise<void>* started) {
    started->set_value();
    ready_future.wait();
    TimeZone tz;
    for (const char* const* np = kTimeZoneNames; *np != nullptr; ++np) {
      EXPECT_TRUE(LoadTimeZone(*np, &tz));
    }
  };

  std::vector<std::thread> threads;
  for (size_t i = 0; i != 256; ++i) {
    std::promise<void> started;
    threads.emplace_back(load_zones, &started);
    started.get_future().wait();
  }

  ready_promise.set_value();

  for (auto& thread : threads) {
    thread.join();
  }
}

TEST(TimeZone, Failures) {
  TimeZone tz;
  EXPECT_FALSE(LoadTimeZone(":America/Los_Angeles", &tz));

  tz = LoadZone("America/Los_Angeles");
  EXPECT_FALSE(LoadTimeZone("Invalid/TimeZone", &tz));
  EXPECT_EQ(system_clock::from_time_t(0),
            MakeTime(1970, 1, 1, 0, 0, 0, tz));  // UTC

  // Ensures that the load still fails on a subsequent attempt.
  tz = LoadZone("America/Los_Angeles");
  EXPECT_FALSE(LoadTimeZone("Invalid/TimeZone", &tz));
  EXPECT_EQ(system_clock::from_time_t(0),
            MakeTime(1970, 1, 1, 0, 0, 0, tz));  // UTC

  // Loading an empty string timezone should fail.
  tz = LoadZone("America/Los_Angeles");
  EXPECT_FALSE(LoadTimeZone("", &tz));
  EXPECT_EQ(system_clock::from_time_t(0),
            MakeTime(1970, 1, 1, 0, 0, 0, tz));  // UTC
}

TEST(StdChronoTimePoint, TimeTAlignment) {
  // Ensures that the Unix epoch and the system clock epoch are an integral
  // number of seconds apart. This simplifies conversions to/from time_t.
  using TP = std::chrono::system_clock::time_point;
  auto diff = TP() - std::chrono::system_clock::from_time_t(0);
  EXPECT_EQ(TP::duration::zero(), diff % std::chrono::seconds(1));
}

TEST(BreakTime, TimePointResolution) {
  using std::chrono::time_point_cast;
  const TimeZone utc = UTCTimeZone();
  const auto t0 = system_clock::from_time_t(0);
  Breakdown bd{};

  bd = BreakTime(time_point_cast<std::chrono::nanoseconds>(t0), utc);
  ExpectTime(bd, 1970, 1, 1, 0, 0, 0, 0, false, "UTC");
  bd = BreakTime(time_point_cast<std::chrono::microseconds>(t0), utc);
  ExpectTime(bd, 1970, 1, 1, 0, 0, 0, 0, false, "UTC");
  bd = BreakTime(time_point_cast<std::chrono::milliseconds>(t0), utc);
  ExpectTime(bd, 1970, 1, 1, 0, 0, 0, 0, false, "UTC");
  bd = BreakTime(time_point_cast<std::chrono::seconds>(t0), utc);
  ExpectTime(bd, 1970, 1, 1, 0, 0, 0, 0, false, "UTC");
  bd = BreakTime(time_point_cast<seconds64>(t0), utc);
  ExpectTime(bd, 1970, 1, 1, 0, 0, 0, 0, false, "UTC");
  bd = BreakTime(time_point_cast<std::chrono::minutes>(t0), utc);
  ExpectTime(bd, 1970, 1, 1, 0, 0, 0, 0, false, "UTC");
  bd = BreakTime(time_point_cast<std::chrono::hours>(t0), utc);
  ExpectTime(bd, 1970, 1, 1, 0, 0, 0, 0, false, "UTC");
}

TEST(BreakTime, LocalTimeInUTC) {
  const Breakdown bd = BreakTime(system_clock::from_time_t(0), UTCTimeZone());
  ExpectTime(bd, 1970, 1, 1, 0, 0, 0, 0, false, "UTC");
  EXPECT_EQ(4, bd.weekday);  // Thursday
}

TEST(BreakTime, LocalTimePosix) {
  // See IEEE Std 1003.1-1988 B.2.3 General Terms, Epoch.
  const Breakdown bd =
      BreakTime(system_clock::from_time_t(536457599), UTCTimeZone());
  ExpectTime(bd, 1986, 12, 31, 23, 59, 59, 0, false, "UTC");
  EXPECT_EQ(3, bd.weekday);  // Wednesday
}

TEST(BreakTime, LocalTimeInNewYork) {
  const TimeZone tz = LoadZone("America/New_York");
  const Breakdown bd = BreakTime(system_clock::from_time_t(45), tz);
  ExpectTime(bd, 1969, 12, 31, 19, 0, 45, -5 * 60 * 60, false, "EST");
  EXPECT_EQ(3, bd.weekday);  // Wednesday
}

TEST(BreakTime, LocalTimeInMTV) {
  const TimeZone tz = LoadZone("America/Los_Angeles");
  const Breakdown bd = BreakTime(system_clock::from_time_t(1380855729), tz);
  ExpectTime(bd, 2013, 10, 3, 20, 2, 9, -7 * 60 * 60, true, "PDT");
  EXPECT_EQ(4, bd.weekday);  // Thursday
}

TEST(BreakTime, LocalTimeInSydney) {
  const TimeZone tz = LoadZone("Australia/Sydney");
  const Breakdown bd = BreakTime(system_clock::from_time_t(90), tz);
  ExpectTime(bd, 1970, 1, 1, 10, 1, 30, 10 * 60 * 60, false, "AEST");
  EXPECT_EQ(4, bd.weekday);  // Thursday
}

TEST(MakeTime, TimePointResolution) {
  const TimeZone utc = UTCTimeZone();
  const time_point<std::chrono::nanoseconds> tp_ns =
      MakeTime(2015, 1, 2, 3, 4, 5, utc);
  EXPECT_EQ("04:05", Format("%M:%E*S", tp_ns, utc));
  const time_point<std::chrono::microseconds> tp_us =
      MakeTime(2015, 1, 2, 3, 4, 5, utc);
  EXPECT_EQ("04:05", Format("%M:%E*S", tp_us, utc));
  const time_point<std::chrono::milliseconds> tp_ms =
      MakeTime(2015, 1, 2, 3, 4, 5, utc);
  EXPECT_EQ("04:05", Format("%M:%E*S", tp_ms, utc));
  const time_point<std::chrono::seconds> tp_s =
      MakeTime(2015, 1, 2, 3, 4, 5, utc);
  EXPECT_EQ("04:05", Format("%M:%E*S", tp_s, utc));
  const time_point<seconds64> tp_s64 =
      MakeTime(2015, 1, 2, 3, 4, 5, utc);
  EXPECT_EQ("04:05", Format("%M:%E*S", tp_s64, utc));

  // These next two require time_point_cast because the conversion from a
  // resolution of seconds (the return value of MakeTime()) to a coarser
  // resolution requires an explicit cast.
  using std::chrono::time_point_cast;
  const time_point<std::chrono::minutes> tp_m =
      time_point_cast<std::chrono::minutes>(MakeTime(2015, 1, 2, 3, 4, 5, utc));
  EXPECT_EQ("04:00", Format("%M:%E*S", tp_m, utc));
  const time_point<std::chrono::hours> tp_h =
      time_point_cast<std::chrono::hours>(MakeTime(2015, 1, 2, 3, 4, 5, utc));
  EXPECT_EQ("00:00", Format("%M:%E*S", tp_h, utc));
}

TEST(MakeTime, Normalization) {
  const TimeZone tz = LoadZone("America/New_York");
  const auto tp = MakeTime(2009, 2, 13, 18, 31, 30, tz);
  EXPECT_EQ(system_clock::from_time_t(1234567890), tp);

  // Now requests for the same time_point but with out-of-range fields.
  EXPECT_EQ(tp, MakeTime(2008, 14, 13, 18, 31, 30, tz));  // month
  EXPECT_EQ(tp, MakeTime(2009, 1, 44, 18, 31, 30, tz));   // day
  EXPECT_EQ(tp, MakeTime(2009, 2, 12, 42, 31, 30, tz));   // hour
  EXPECT_EQ(tp, MakeTime(2009, 2, 13, 17, 91, 30, tz));   // minute
  EXPECT_EQ(tp, MakeTime(2009, 2, 13, 18, 30, 90, tz));   // second
}

TEST(TimeZoneEdgeCase, AmericaNewYork) {
  const TimeZone tz = LoadZone("America/New_York");

  // Spring 1:59:59 -> 3:00:00
  auto tp = MakeTime(2013, 3, 10, 1, 59, 59, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 3, 10, 1, 59, 59, -5 * 3600, false, "EST");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 3, 10, 3, 0, 0, -4 * 3600, true, "EDT");

  // Fall 1:59:59 -> 1:00:00
  tp = MakeTime(2013, 11, 3, 1, 59, 59, tz);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 11, 3, 1, 59, 59, -4 * 3600, true, "EDT");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 11, 3, 1, 0, 0, -5 * 3600, false, "EST");
}

TEST(TimeZoneEdgeCase, AmericaLosAngeles) {
  const TimeZone tz = LoadZone("America/Los_Angeles");

  // Spring 1:59:59 -> 3:00:00
  auto tp = MakeTime(2013, 3, 10, 1, 59, 59, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 3, 10, 1, 59, 59, -8 * 3600, false, "PST");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 3, 10, 3, 0, 0, -7 * 3600, true, "PDT");

  // Fall 1:59:59 -> 1:00:00
  tp = MakeTime(2013, 11, 3, 1, 59, 59, tz);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 11, 3, 1, 59, 59, -7 * 3600, true, "PDT");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 11, 3, 1, 0, 0, -8 * 3600, false, "PST");
}

TEST(TimeZoneEdgeCase, ArizonaNoTransition) {
  const TimeZone tz = LoadZone("America/Phoenix");

  // No transition in Spring.
  auto tp = MakeTime(2013, 3, 10, 1, 59, 59, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 3, 10, 1, 59, 59, -7 * 3600, false, "MST");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 3, 10, 2, 0, 0, -7 * 3600, false, "MST");

  // No transition in Fall.
  tp = MakeTime(2013, 11, 3, 1, 59, 59, tz);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 11, 3, 1, 59, 59, -7 * 3600, false, "MST");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 11, 3, 2, 0, 0, -7 * 3600, false, "MST");
}

TEST(TimeZoneEdgeCase, AsiaKathmandu) {
  const TimeZone tz = LoadZone("Asia/Kathmandu");

  // A non-DST offset change from +0530 to +0545
  //
  //   504901799 == Tue, 31 Dec 1985 23:59:59 +0530 (IST)
  //   504901800 == Wed,  1 Jan 1986 00:15:00 +0545 (NPT)
  auto tp = MakeTime(1985, 12, 31, 23, 59, 59, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 1985, 12, 31, 23, 59, 59, 5.5 * 3600, false, "IST");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 1986, 1, 1, 0, 15, 0, 5.75 * 3600, false, "NPT");
}

TEST(TimeZoneEdgeCase, PacificChatham) {
  const TimeZone tz = LoadZone("Pacific/Chatham");

  // One-hour DST offset changes, but at atypical values
  //
  //   1365256799 == Sun,  7 Apr 2013 03:44:59 +1345 (CHADT)
  //   1365256800 == Sun,  7 Apr 2013 02:45:00 +1245 (CHAST)
  auto tp = MakeTime(2013, 4, 7, 3, 44, 59, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 4, 7, 3, 44, 59, 13.75 * 3600, true, "CHADT");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 4, 7, 2, 45, 0, 12.75 * 3600, false, "CHAST");

  //   1380376799 == Sun, 29 Sep 2013 02:44:59 +1245 (CHAST)
  //   1380376800 == Sun, 29 Sep 2013 03:45:00 +1345 (CHADT)
  tp = MakeTime(2013, 9, 29, 2, 44, 59, tz);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 9, 29, 2, 44, 59, 12.75 * 3600, false, "CHAST");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 9, 29, 3, 45, 0, 13.75 * 3600, true, "CHADT");
}

TEST(TimeZoneEdgeCase, AustraliaLordHowe) {
  const TimeZone tz = LoadZone("Australia/Lord_Howe");

  // Half-hour DST offset changes
  //
  //   1365260399 == Sun,  7 Apr 2013 01:59:59 +1100 (LHDT)
  //   1365260400 == Sun,  7 Apr 2013 01:30:00 +1030 (LHST)
  auto tp = MakeTime(2013, 4, 7, 1, 59, 59, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 4, 7, 1, 59, 59, 11 * 3600, true, "LHDT");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 4, 7, 1, 30, 0, 10.5 * 3600, false, "LHST");

  //   1380986999 == Sun,  6 Oct 2013 01:59:59 +1030 (LHST)
  //   1380987000 == Sun,  6 Oct 2013 02:30:00 +1100 (LHDT)
  tp = MakeTime(2013, 10, 6, 1, 59, 59, tz);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 10, 6, 1, 59, 59, 10.5 * 3600, false, "LHST");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2013, 10, 6, 2, 30, 0, 11 * 3600, true, "LHDT");
}

TEST(TimeZoneEdgeCase, PacificApia) {
  const TimeZone tz = LoadZone("Pacific/Apia");

  // At the end of December 2011, Samoa jumped forward by one day,
  // skipping 30 December from the local calendar, when the nation
  // moved to the west of the International Date Line.
  //
  // A one-day, non-DST offset change
  //
  //   1325239199 == Thu, 29 Dec 2011 23:59:59 -1000 (SDT)
  //   1325239200 == Sat, 31 Dec 2011 00:00:00 +1400 (WSDT)
  auto tp = MakeTime(2011, 12, 29, 23, 59, 59, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2011, 12, 29, 23, 59, 59, -10 * 3600, true, "SDT");
  EXPECT_EQ(363, bd.yearday);
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2011, 12, 31, 0, 0, 0, 14 * 3600, true, "WSDT");
  EXPECT_EQ(365, bd.yearday);  // What else could 2011/12/31 be!?
}

TEST(TimeZoneEdgeCase, AfricaCairo) {
  const TimeZone tz = LoadZone("Africa/Cairo");

  // An interesting case of midnight not existing.
  //
  //   1400191199 == Thu, 15 May 2014 23:59:59 +0200 (EET)
  //   1400191200 == Fri, 16 May 2014 01:00:00 +0300 (EEST)
  auto tp = MakeTime(2014, 5, 15, 23, 59, 59, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2014, 5, 15, 23, 59, 59, 2 * 3600, false, "EET");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2014, 5, 16, 1, 0, 0, 3 * 3600, true, "EEST");
}

TEST(TimeZoneEdgeCase, AfricaMonrovia) {
  const TimeZone tz = LoadZone("Africa/Monrovia");

  // Strange offset change -00:44:30 -> +00:00:00 (non-DST)
  //
  //   73529069 == Sun, 30 Apr 1972 23:59:59 -0044 (LRT)
  //   73529070 == Mon,  1 May 1972 00:44:30 +0000 (GMT)
  auto tp = MakeTime(1972, 4, 30, 23, 59, 59, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 1972, 4, 30, 23, 59, 59, -44.5 * 60, false, "LRT");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 1972, 5, 1, 0, 44, 30, 0 * 60, false, "GMT");
}

TEST(TimeZoneEdgeCase, AmericaJamaica) {
  // Jamaica discontinued DST transitions in 1983, and is now at a
  // constant -0500.  This makes it an interesting edge-case target.
  // Note that the 32-bit times used in a (tzh_version == 0) zoneinfo
  // file cannot represent the abbreviation-only transition of 1890,
  // so we ignore the abbreviation by expecting what we received.
  const TimeZone tz = LoadZone("America/Jamaica");

  // Before the first transition.
  auto tp = MakeTime(1889, 12, 31, 0, 0, 0, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 1889, 12, 31, 0, 0, 0, -18431, false, bd.abbr);

  // Over the first (abbreviation-change only) transition.
  //   -2524503170 == Tue, 31 Dec 1889 23:59:59 -0507 (LMT)
  //   -2524503169 == Wed,  1 Jan 1890 00:00:00 -0507 (KMT)
  tp = MakeTime(1889, 12, 31, 23, 59, 59, tz);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 1889, 12, 31, 23, 59, 59, -18431, false, bd.abbr);
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 1890, 1, 1, 0, 0, 0, -18431, false, "KMT");

  // Over the last (DST) transition.
  //     436341599 == Sun, 30 Oct 1983 01:59:59 -0400 (EDT)
  //     436341600 == Sun, 30 Oct 1983 01:00:00 -0500 (EST)
  tp = MakeTime(1983, 10, 30, 1, 59, 59, tz);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 1983, 10, 30, 1, 59, 59, -4 * 3600, true, "EDT");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 1983, 10, 30, 1, 0, 0, -5 * 3600, false, "EST");

  // After the last transition.
  tp = MakeTime(1983, 12, 31, 23, 59, 59, tz);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 1983, 12, 31, 23, 59, 59, -5 * 3600, false, "EST");
}

TEST(TimeZoneEdgeCase, WET) {
  // Cover some non-existent times within forward transitions.
  const TimeZone tz = LoadZone("WET");

  // Before the first transition.
  auto tp = MakeTime(1977, 1, 1, 0, 0, 0, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 1977, 1, 1, 0, 0, 0, 0, false, "WET");

  // Over the first transition.
  //     228877199 == Sun,  3 Apr 1977 00:59:59 +0000 (WET)
  //     228877200 == Sun,  3 Apr 1977 02:00:00 +0100 (WEST)
  tp = MakeTime(1977, 4, 3, 0, 59, 59, tz);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 1977, 4, 3, 0, 59, 59, 0, false, "WET");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 1977, 4, 3, 2, 0, 0, 1 * 3600, true, "WEST");

  // A non-existent time within the first transition.
  TimeInfo ti1 = MakeTimeInfo(1977, 4, 3, 1, 15, 0, tz);
  EXPECT_FALSE(ti1.normalized);
  EXPECT_EQ(TimeInfo::Kind::SKIPPED, ti1.kind);
  bd = BreakTime(ti1.pre, tz);
  ExpectTime(bd, 1977, 4, 3, 2, 15, 0, 1 * 3600, true, "WEST");
  bd = BreakTime(ti1.trans, tz);
  ExpectTime(bd, 1977, 4, 3, 2, 0, 0, 1 * 3600, true, "WEST");
  bd = BreakTime(ti1.post, tz);
  ExpectTime(bd, 1977, 4, 3, 0, 15, 0, 0 * 3600, false, "WET");

  // A non-existent time within the second forward transition.
  TimeInfo ti2 = MakeTimeInfo(1978, 4, 2, 1, 15, 0, tz);
  EXPECT_FALSE(ti2.normalized);
  EXPECT_EQ(TimeInfo::Kind::SKIPPED, ti2.kind);
  bd = BreakTime(ti2.pre, tz);
  ExpectTime(bd, 1978, 4, 2, 2, 15, 0, 1 * 3600, true, "WEST");
  bd = BreakTime(ti2.trans, tz);
  ExpectTime(bd, 1978, 4, 2, 2, 0, 0, 1 * 3600, true, "WEST");
  bd = BreakTime(ti2.post, tz);
  ExpectTime(bd, 1978, 4, 2, 0, 15, 0, 0 * 3600, false, "WET");
}

TEST(TimeZoneEdgeCase, FixedOffsets) {
  const TimeZone gmtm5 = LoadZone("Etc/GMT+5");  // -0500
  auto tp = MakeTime(1970, 1, 1, 0, 0, 0, gmtm5);
  Breakdown bd = BreakTime(tp, gmtm5);
  ExpectTime(bd, 1970, 1, 1, 0, 0, 0, -5 * 3600, false, "GMT+5");
  EXPECT_EQ(system_clock::from_time_t(5 * 3600), tp);

  const TimeZone gmtp5 = LoadZone("Etc/GMT-5");  // +0500
  tp = MakeTime(1970, 1, 1, 0, 0, 0, gmtp5);
  bd = BreakTime(tp, gmtp5);
  ExpectTime(bd, 1970, 1, 1, 0, 0, 0, 5 * 3600, false, "GMT-5");
  EXPECT_EQ(system_clock::from_time_t(-5 * 3600), tp);
}

TEST(TimeZoneEdgeCase, NegativeYear) {
  // Tests transition from year 0 (aka 1BCE) to year -1.
  const TimeZone tz = UTCTimeZone();
  auto tp = MakeTime(0, 1, 1, 0, 0, 0, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 0, 1, 1, 0, 0, 0, 0 * 3600, false, "UTC");
  EXPECT_EQ(6, bd.weekday);
  tp -= std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, -1, 12, 31, 23, 59, 59, 0 * 3600, false, "UTC");
  EXPECT_EQ(5, bd.weekday);
}

TEST(TimeZoneEdgeCase, UTC32bitLimit) {
  const TimeZone tz = UTCTimeZone();

  // Limits of signed 32-bit time_t
  //
  //   2147483647 == Tue, 19 Jan 2038 03:14:07 +0000 (UTC)
  //   2147483648 == Tue, 19 Jan 2038 03:14:08 +0000 (UTC)
  auto tp = MakeTime(2038, 1, 19, 3, 14, 7, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 2038, 1, 19, 3, 14, 7, 0 * 3600, false, "UTC");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 2038, 1, 19, 3, 14, 8, 0 * 3600, false, "UTC");
}

TEST(TimeZoneEdgeCase, UTC5DigitYear) {
  const TimeZone tz = UTCTimeZone();

  // Rollover to 5-digit year
  //
  //   253402300799 == Fri, 31 Dec 9999 23:59:59 +0000 (UTC)
  //   253402300800 == Sat,  1 Jan 1000 00:00:00 +0000 (UTC)
  auto tp = MakeTime(9999, 12, 31, 23, 59, 59, tz);
  Breakdown bd = BreakTime(tp, tz);
  ExpectTime(bd, 9999, 12, 31, 23, 59, 59, 0 * 3600, false, "UTC");
  tp += std::chrono::seconds(1);
  bd = BreakTime(tp, tz);
  ExpectTime(bd, 10000, 1, 1, 0, 0, 0, 0 * 3600, false, "UTC");
}

// These tests are disabled because they require 64-bit civil years.
#if 0
TEST(TimeZoneEdgeCase, East64bitLimit) {
  // For zones with positive offsets we cannot really get all the way to the
  // maximal time_point as anything closer than the offset will (currently)
  // result in an internal integer overflow. (Check 15:30:08 with -ftrapv.)
  const TimeZone tz = LoadZone("Etc/GMT-14");
  time_point<seconds64> tp = MakeTime(292277026596, 12, 4, 15, 30, 7, tz);
  EXPECT_EQ(std::numeric_limits<seconds64::rep>::max() - 14 * 3600,
            tp.time_since_epoch().count());
}

TEST(TimeZoneEdgeCase, West64bitLimit) {
  // For zones with negative offsets we cannot really get all the way to the
  // minimal time_point as anything closer than the offset will (currently)
  // result in an internal integer overflow. (Check 08:29:51 with -ftrapv.)
  const TimeZone tz = LoadZone("Etc/GMT+12");
  time_point<seconds64> tp = MakeTime(-292277022657, 1, 27, 8, 29, 52, tz);
  EXPECT_EQ(std::numeric_limits<seconds64::rep>::min() + 12 * 3600,
            tp.time_since_epoch().count());
}
#endif

}  // namespace cctz
