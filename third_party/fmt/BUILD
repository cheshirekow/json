package(default_visibility=["//visibility:public"])
licenses(["permissive"])

cc_library(
    name="fmt",
    includes=["include"],
    hdrs=["include/fmt/core.h",
          "include/fmt/format-inl.h",
          "include/fmt/format.h",
          "include/fmt/ostream.h",
          "include/fmt/posix.h",
          "include/fmt/printf.h",
          "include/fmt/ranges.h",
          "include/fmt/time.h"],
    srcs=["src/format.cc", "src/posix.cc"],
    strip_include_prefix="include/"
)
