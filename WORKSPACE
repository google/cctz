workspace(name = "com_googlesource_code_cctz")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# GoogleTest/GoogleMock framework. Used by most unit-tests.
http_archive(
    name = "com_google_googletest",
    sha256 = "94c634d499558a76fa649edb13721dce6e98fb1e7018dfaeba3cd7a083945e91",
    strip_prefix = "googletest-release-1.10.0",
    urls = ["https://github.com/google/googletest/archive/release-1.10.0.zip"],
)

# Google Benchmark library.
http_archive(
    name = "com_github_google_benchmark",
    sha256 = "3e266b49f73ee08625837ea5b1fabc4056b7f5e809b29c49670527326f4f4379",
    strip_prefix = "benchmark-1.5.1",
    urls = ["https://github.com/google/benchmark/archive/v1.5.1.zip"],
)

# C++ rules for Bazel.
http_archive(
    name = "rules_cc",
    sha256 = "fa42eade3cad9190c2a6286a6213f07f1a83d26d9f082d56f526d014c6ea7444",
    strip_prefix = "rules_cc-02becfef8bc97bda4f9bb64e153f1b0671aec4ba",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_cc/archive/02becfef8bc97bda4f9bb64e153f1b0671aec4ba.zip",
        "https://github.com/bazelbuild/rules_cc/archive/02becfef8bc97bda4f9bb64e153f1b0671aec4ba.zip",
    ],
)
