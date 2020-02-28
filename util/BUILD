package(default_visibility=["//visibility:public"])

cc_library(
    name="util",
    hdrs=["binary_literal.h",
          "bitset.h",
          "container_of.h",
          "fixed_string_stream.h",
          "hash_tpl.h",
          "inline_math.h",
          "linked.h",
          "linked.hpp",
          "null_out.h",
          "range.hpp",
          "redblack.h",
          "stack_trace.h",
          "set_operations.hpp",
          "timespec.h",
          "type_string.h"],
    srcs=["linked.cc",
          "redblack.cc",
          "stack_trace.cc"],
)

cc_test(name="bitset_test",
        deps=[":util",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"],
        srcs=["bitset_test.cc"])

cc_test(name="linked_test",
        deps=[":util",
              "//third_party/fmt:fmt",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"],
        srcs=["linked_test.cc"])

cc_test(name="redblack_test",
        deps=[":util",
              "//third_party/fmt:fmt",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"],
        srcs=["redblack_test.cc"])

cc_test(name="type_string_test",
        deps=[":util",
              "//third_party/fmt:fmt",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"],
        srcs=["type_string_test.cc"])

cc_test(name="fixed_string_stream_test",
        deps=[":util",
              "//third_party/fmt:fmt",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"],
        srcs=["fixed_string_stream_test.cc"])

cc_library(
    name="svg_artist",
    hdrs=["svg_artist.h"],
    srcs=["svg_artist.cc"],
    deps=["//third_party/fmt:fmt",
          "@system//:eigen3"],
)
