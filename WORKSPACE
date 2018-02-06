workspace(name = "com_googlesource_code_cctz")

# GoogleTest/GoogleMock framework. Used by most unit-tests.
http_archive(
    name = "com_google_googletest",
    urls = ["https://github.com/google/googletest/archive/master.zip"],
    strip_prefix = "googletest-master",
)

# Google Benchmark library.
new_http_archive(
    name = "com_google_benchmark",
    urls = ["https://github.com/google/benchmark/archive/master.zip"],
    strip_prefix = "benchmark-master",
    build_file_content =
"""
config_setting(
    name = "windows",
    values = {
        "cpu": "x64_windows",
    },
)

cc_library(
    name = "benchmark",
    srcs = glob([
        "src/*.h",
        "src/*.cc",
    ]),
    hdrs = glob(["include/benchmark/*.h"]),
    copts = [
        "-DHAVE_STD_REGEX",
    ],
    includes = [
        "include",
    ],
    linkopts = select({
        ":windows": ["-defaultlib:shlwapi.lib"],
        "//conditions:default": ["-pthread"],
    }),
    visibility = ["//visibility:public"],
)
"""
)
