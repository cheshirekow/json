package(default_visibility=["//visibility:public"])

cc_library(
    name="argue",
    srcs=["argue.cc",
          "argue_glog.cc"],
    hdrs=["argue.h",
          "argue_glog.h",
          "argue_macros.h", ],
    deps=["//third_party/fmt:fmt",
          "//util:util",
          "@system//:glog"],
)

cc_binary(
    name="argue-demo",
    srcs=["demo.cc"],
    deps=[":argue"],
)

cc_test(
    name="argue-assert_test",
    srcs=["test/assert_test.cc"],
    deps=[":argue",
          "//third_party/googletest:gtest",
          "//third_party/googletest:gtest_main"]
)

cc_test(
    name="argue-arg_test",
    srcs=["test/arg_test.cc"],
    deps=[":argue",
          "//third_party/googletest:gtest",
          "//third_party/googletest:gtest_main"]
)

cc_test(
    name="argue-parse_test",
    srcs=["test/parse_test.cc"],
    deps=[":argue",
          "//third_party/googletest:gtest",
          "//third_party/googletest:gtest_main"]
)

cc_test(
    name="argue-util_test",
    srcs=["test/util_test.cc"],
    deps=[":argue",
          "//third_party/googletest:gtest",
          "//third_party/googletest:gtest_main"]
)

# NOTE(josh): see
# https://docs.bazel.build/versions/master/skylark/testing.html
sh_test(
    name="argue-execution_test",
    srcs=["test/execution_tests.py"],
    args=["--exe-path", "$(location :argue-demo)"],
    data=[":argue-demo"]
)
