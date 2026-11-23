load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_shell//shell:sh_test.bzl", "sh_test")


def cc_test_with_testdata(name, data=[], env={}, **kwargs):
    bin_name = name + "_bin"

    # Extract test-only attributes so cc_binary does not reject them.
    test_kwargs = {}
    for attr in ["size", "timeout", "flaky", "shard_count", "local", "tags"]:
        if attr in kwargs:
            test_kwargs[attr] = kwargs.pop(attr)

    cc_binary(name=bin_name, testonly=True, **kwargs)

    test_data = (
        list(data)
        + [":" + bin_name]
        + native.glob(["testdata/zoneinfo/**"])
    )

    test_env = dict(env)
    test_env.update(
        {
            "TEST_BIN_RLOCATION": (
                "$(rlocationpath :" + bin_name + ")"
            ),
            "ZONEINFO_ANCHOR_RLOCATION": (
                "$(rlocationpath testdata/zoneinfo/UTC)"
            ),
        }
    )

    sh_test(
        name=name,
        srcs=["with_testdata.sh"],
        data=test_data,
        env=test_env,
        deps=["@bazel_tools//tools/bash/runfiles"],
        **test_kwargs
    )
