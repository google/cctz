# GoogleTest/GoogleMock framework. Used by most unit-tests.
http_archive(
     name = "com_google_googletest",
     urls = ["https://github.com/google/googletest/archive/master.zip"],
     strip_prefix = "googletest-master",
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
