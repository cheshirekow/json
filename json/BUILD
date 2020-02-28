package(default_visibility=["//visibility:public"])

cc_library(name="json",
           hdrs=glob(["*.h"]),
           srcs=["builder.cc",
                 "item.cc",
                 "json.cc",
                 "parse.cc",
                 "pipeline.cc",
                 "type_registry.cc",
                 "util.cc",
                 "variant.cc"],
           deps=["//third_party/fmt:fmt",
                 "//third_party:re2",
                 "//util:util",
                 "@system//:glog"])

cc_binary(name="json.exe",
          srcs=["main.cc"],
          deps=["//argue:argue", ":json"])

py_binary(
    name="json_gen",
    srcs=["json_gen.py"],
    data=["json_gen.h.tpl",
          "json_gen.cc.tpl"],
    legacy_create_init=False,
)

genrule(
    name="test_types",
    srcs=["test/test_types.py"],
    outs=["test/test_types.h",
          "test/test_types.cc"],
    cmd=("./$(location :json_gen)" +
         " --outdir $(@D)/test $(location test/test_types.py)"),
    tools=[":json_gen"],
)

cc_test(name="lexer_test",
        srcs=["test/lexer_test.cc"],
        deps=[":json",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"])

cc_test(name="parser_test",
        srcs=["test/parser_test.cc"],
        deps=[":json",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"])

cc_test(name="builder_test",
        srcs=["test/builder_test.cc"],
        deps=[":json",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"])

cc_test(name="variant_test",
        srcs=["test/variant_test.cc"],
        deps=[":json",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"])

cc_test(name="stream_test",
        srcs=["test/stream_test.cc"],
        deps=[":json",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"])

cc_test(name="macro_test",
        srcs=["test/macro_test.cc"],
        deps=[":json",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"])

cc_test(name="stream_gen_test",
        srcs=["test/stream_gen_test.h",
              "test/stream_gen_test.cc",
              "test/test_types.h",
              "test/test_types.cc"],
        deps=[":json",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"])

cc_test(name="type-registry-test",
        srcs=["test/type_registry_test.cc"],
        deps=[":json",
              "//third_party/googletest:gtest",
              "//third_party/googletest:gtest_main"])

py_test(
    name="frontend-tests",
    srcs=["test/frontend_tests.py"],
    main="test/frontend_tests.py",
    data=[":json.exe",
          "test/frontend_tests.md"],
    args=["--exe-path", "$(location :json.exe)"])
