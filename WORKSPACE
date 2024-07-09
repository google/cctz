workspace(name = "com_googlesource_code_cctz")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# GoogleTest/GoogleMock framework. Used by most unit-tests.
http_archive(
    name = "com_google_googletest",
    sha256 = "8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7",
    strip_prefix = "googletest-1.14.0",
    urls = ["https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz"],
)

# Google Benchmark library.
http_archive(
    name = "com_github_google_benchmark",
    sha256 = "3e7059b6b11fb1bbe28e33e02519398ca94c1818874ebed18e504dc6f709be45",
    strip_prefix = "benchmark-1.8.4",
    urls = ["https://github.com/google/benchmark/archive/refs/tags/v1.8.4.tar.gz"],
)

# Bazel platform rules.
http_archive(
    name = "platforms",
    sha256 = "8150406605389ececb6da07cbcb509d5637a3ab9a24bc69b1101531367d89d74",
    urls = ["https://github.com/bazelbuild/platforms/releases/download/0.0.8/platforms-0.0.8.tar.gz"],
)
