new_git_repository(
    name = "gtest",
    remote = "https://github.com/google/googletest.git",
    commit = "de411c3e80120f8dcc2a3f4f62f3ca692c0431d7",
    build_file_content =
"""
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
"""
)

new_git_repository(
    name = "benchmark",
    remote = "https://github.com/google/benchmark.git",
    commit = "cb8a0cc10f8b634fd554251ae086da522b58f50e",
    build_file_content =
"""
cc_library(
    name = "benchmark",
    srcs = glob(["src/*.h", "src/*.cc"]),
    hdrs = glob(["include/benchmark/*.h"]),
    includes = [
        "include",
    ],
    visibility = ["//visibility:public"],
    copts = [
        "-DHAVE_STD_REGEX"
    ],
)
"""
)
