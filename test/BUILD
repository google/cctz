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
    name = "cnv_test",
    size = "small",
    srcs = ["cnv_test.cc"],
    deps = [
        "@gtest//:gtest",
        "//src:cctz",
    ],
)

cc_test(
    name = "fmt_test",
    size = "small",
    srcs = ["fmt_test.cc"],
    deps = [
        "@gtest//:gtest",
        "//src:cctz",
    ],
)
