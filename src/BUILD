cc_library(
    name = "cctz",
    srcs = [
        "cctz_cnv.cc",
        "cctz_fmt.cc",
        "cctz_if.cc",
        "cctz_if.h",
        "cctz_impl.cc",
        "cctz_impl.h",
        "cctz_info.cc",
        "cctz_info.h",
        "cctz_libc.cc",
        "cctz_libc.h",
        "cctz_posix.cc",
        "cctz_posix.h",
        "tzfile.h",
    ],
    hdrs = ["cctz.h"],
    linkopts = [
        "-lm",
        "-lpthread",
    ],
    visibility = ["//visibility:public"],
)
