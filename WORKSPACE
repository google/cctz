workspace(name = "com_googlesource_code_cctz")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# GoogleTest/GoogleMock framework. Used by most unit-tests.
http_archive(
    name = "com_google_googletest",
    sha256 = "353571c2440176ded91c2de6d6cd88ddd41401d14692ec1f99e35d013feda55a",
    strip_prefix = "googletest-release-1.11.0",
    urls = ["https://github.com/google/googletest/archive/release-1.11.0.zip"],
)

# Google Benchmark library.
http_archive(
    name = "com_github_google_benchmark",
    sha256 = "30f2e5156de241789d772dd8b130c1cb5d33473cc2f29e4008eab680df7bd1f0",
    strip_prefix = "benchmark-1.5.5",
    urls = ["https://github.com/google/benchmark/archive/v1.5.5.zip"],
)

# C++ rules for Bazel.
http_archive(
    name = "rules_cc",
    sha256 = "f3908cb40a6577ab0d1ef9e00052739bf69b4313fa7d20b4d61d4521ea67abf3",
    strip_prefix = "rules_cc-ab5395627c80e025e824bd005d41f96b20618b9d",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/ab5395627c80e025e824bd005d41f96b20618b9d.zip"],
)
