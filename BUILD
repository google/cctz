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

### libraries

cc_library(
    name = "civil_time",
    hdrs = [
        "include/civil_time.h",
    ],
    includes = ["include"],
    textual_hdrs = ["include/civil_time_detail.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "time_zone",
    srcs = [
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
    ],
    hdrs = [
        "include/time_zone.h",
    ],
    includes = ["include"],
    linkopts = [
        "-lm",
        "-lpthread",
    ],
    visibility = ["//visibility:public"],
    deps = [":civil_time"],
)

cc_library(
    name = "cctz_v1",
    hdrs = [
        "src/cctz.h",
    ],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        ":civil_time",
        ":time_zone",
    ],
)

### tests

# Builds the Google Test source that was fetched from another repository.
cc_library(
    name = "gtest",
    srcs = glob(
        [
            "google*/src/*.cc",
        ],
        exclude = glob([
            "google*/src/*-all.cc",
            "googlemock/src/gmock_main.cc",
        ]),
    ),
    hdrs = glob(["*/include/**/*.h"]),
    includes = [
        "googlemock/",
        "googlemock/include",
        "googletest/",
        "googletest/include",
    ],
    linkopts = ["-pthread"],
    textual_hdrs = ["googletest/src/gtest-internal-inl.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "civil_time_test",
    size = "small",
    srcs = ["src/civil_time_test.cc"],
    deps = [
        "@gtest//:gtest",
        ":civil_time",
    ],
)

cc_test(
    name = "time_zone_format_test",
    size = "small",
    srcs = ["src/time_zone_format_test.cc"],
    deps = [
        "@gtest//:gtest",
        ":civil_time",
        ":time_zone",
    ],
)

cc_test(
    name = "time_zone_lookup_test",
    size = "small",
    srcs = ["src/time_zone_lookup_test.cc"],
    deps = [
        "@gtest//:gtest",
        ":civil_time",
        ":time_zone",
    ],
)

cc_test(
    name = "cctz_v1_test",
    size = "small",
    srcs = ["src/cctz_v1_test.cc"],
    defines = ["CCTZ_ACK_V1_DEPRECATION=1"],
    deps = [
        "@gtest//:gtest",
        ":cctz_v1",
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
    deps = [
        ":civil_time",
        ":time_zone",
    ],
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
    deps = [
        ":civil_time",
        ":time_zone",
    ],
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
    srcs = ["src/time_tool.cc"],
    deps = [
        ":civil_time",
        ":time_zone",
    ],
)
