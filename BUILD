# Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

licenses(["notice"])  # Apache License

### libraries

cc_library(
    name = "civil_time",
    srcs = ["src/civil_time_detail.cc"],
    hdrs = [
        "include/cctz/civil_time.h",
    ],
    includes = ["include"],
    textual_hdrs = ["include/cctz/civil_time_detail.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "time_zone",
    srcs = [
        "src/time_zone_fixed.cc",
        "src/time_zone_fixed.h",
        "src/time_zone_format.cc",
        "src/time_zone_if.cc",
        "src/time_zone_if.h",
        "src/time_zone_impl.cc",
        "src/time_zone_impl.h",
        "src/time_zone_info.cc",
        "src/time_zone_info.h",
        "src/time_zone_libc.cc",
        "src/time_zone_libc.h",
        "src/time_zone_lookup.cc",
        "src/time_zone_posix.cc",
        "src/time_zone_posix.h",
        "src/tzfile.h",
        "src/zone_info_source.cc",
    ],
    hdrs = [
        "include/cctz/time_zone.h",
        "include/cctz/zone_info_source.h",
    ],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":civil_time"],
)

### tests

cc_test(
    name = "civil_time_test",
    size = "small",
    srcs = ["src/civil_time_test.cc"],
    deps = [
        ":civil_time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "time_zone_format_test",
    size = "small",
    srcs = ["src/time_zone_format_test.cc"],
    deps = [
        ":civil_time",
        ":time_zone",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "time_zone_lookup_test",
    size = "small",
    srcs = ["src/time_zone_lookup_test.cc"],
    deps = [
        ":civil_time",
        ":time_zone",
        "@com_google_googletest//:gtest_main",
    ],
)

### benchmarks

cc_binary(
    name = "benchmarks",
    testonly = 1,
    srcs = [
        "src/benchmarks.cc",
        "src/time_zone_if.h",
        "src/time_zone_impl.h",
        "src/time_zone_info.h",
        "src/tzfile.h",
    ],
    linkstatic = 1,
    deps = [
        ":civil_time",
        ":time_zone",
        "@com_google_benchmark//:benchmark",
    ],
)

### examples

cc_binary(
    name = "classic",
    srcs = ["examples/classic.cc"],
)

cc_binary(
    name = "epoch_shift",
    srcs = ["examples/epoch_shift.cc"],
)

cc_binary(
    name = "example1",
    srcs = ["examples/example1.cc"],
    deps = [
        ":civil_time",
        ":time_zone",
    ],
)

cc_binary(
    name = "example2",
    srcs = ["examples/example2.cc"],
    deps = [":time_zone"],
)

cc_binary(
    name = "example3",
    srcs = ["examples/example3.cc"],
    deps = [
        ":civil_time",
        ":time_zone",
    ],
)

cc_binary(
    name = "example4",
    srcs = ["examples/example4.cc"],
    deps = [
        ":civil_time",
        ":time_zone",
    ],
)

cc_binary(
    name = "hello",
    srcs = ["examples/hello.cc"],
    deps = [
        ":civil_time",
        ":time_zone",
    ],
)

### binaries

cc_binary(
    name = "time_tool",
    srcs = [
        "src/time_tool.cc",
        "src/time_zone_if.h",
        "src/time_zone_impl.h",
        "src/time_zone_info.h",
        "src/tzfile.h",
    ],
    deps = [
        ":civil_time",
        ":time_zone",
    ],
)
