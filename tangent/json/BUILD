package(default_visibility = ["//visibility:public"])

cc_library(
  name = "json",
  srcs = [
    "builder.cc",
    "item.cc",
    "json.cc",
    "parse.cc",
    "pipeline.cc",
    "type_registry.cc",
    "util.cc",
    "variant.cc",
  ],
  hdrs = glob(["*.h"]),
  deps = [
    "//tangent/util",
    "@system//:fmt",
    "@system//:glog",
    "@system//:re2",
  ],
)

cc_binary(
  name = "json.exe",
  srcs = ["main.cc"],
  deps = [
    ":json",
    "//argue",
  ],
)

py_binary(
  name = "json_gen",
  srcs = ["json_gen.py"],
  data = [
    "json_gen.cc.tpl",
    "json_gen.h.tpl",
  ],
  legacy_create_init = False,
)

genrule(
  name = "test_types",
  srcs = ["test/test_types.py"],
  outs = [
    "test/test_types.h",
    "test/test_types.cc",
  ],
  cmd = ("./$(location :json_gen)" +
         " --outdir $(@D)/test $(location test/test_types.py)"),
  tools = [":json_gen"],
)

cc_test(
  name = "lexer_test",
  srcs = ["test/lexer_test.cc"],
  deps = [
    ":json",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "parser_test",
  srcs = ["test/parser_test.cc"],
  deps = [
    ":json",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "builder_test",
  srcs = ["test/builder_test.cc"],
  deps = [
    ":json",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "variant_test",
  srcs = ["test/variant_test.cc"],
  deps = [
    ":json",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "stream_test",
  srcs = ["test/stream_test.cc"],
  deps = [
    ":json",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "macro_test",
  srcs = ["test/macro_test.cc"],
  deps = [
    ":json",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "stream_gen_test",
  srcs = [
    "test/stream_gen_test.cc",
    "test/stream_gen_test.h",
    "test/test_types.cc",
    "test/test_types.h",
  ],
  deps = [
    ":json",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

cc_test(
  name = "type-registry-test",
  srcs = ["test/type_registry_test.cc"],
  deps = [
    ":json",
    "//third_party/googletest:gtest",
    "//third_party/googletest:gtest_main",
  ],
)

py_test(
  name = "frontend-tests",
  srcs = ["test/frontend_tests.py"],
  args = [
    "--exe-path",
    "$(location :json.exe)",
  ],
  data = [
    "test/frontend_tests.md",
    ":json.exe",
  ],
  main = "test/frontend_tests.py",
)
