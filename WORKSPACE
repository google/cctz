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

# Bazel platform rules.
http_archive(
    name = "platforms",
    sha256 = "b601beaf841244de5c5a50d2b2eddd34839788000fa1be4260ce6603ca0d8eb7",
    strip_prefix = "platforms-98939346da932eef0b54cf808622f5bb0928f00b",
    urls = ["https://github.com/bazelbuild/platforms/archive/98939346da932eef0b54cf808622f5bb0928f00b.zip"],
)
