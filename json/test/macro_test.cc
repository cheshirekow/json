// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <array>
#include <cstdint>
#include <list>
#include <map>
#include <vector>

#include <gtest/gtest.h>

template <class P, class M>
size_t OffsetOf(const M P::*member) {
  return (size_t) & (reinterpret_cast<P*>(0)->*member);
}

template <class P, class M>
P* ContainerOf(M* ptr, const M P::*member) {
  return (P*)((char*)ptr - OffsetOf(member));
}

// See https://groups.google.com/forum/#!topic/comp.std.c/d-6Mj5Lko_s
// for invention of the technique
#define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                 _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26,  \
                 _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38,  \
                 _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50,  \
                 _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62,  \
                 _63, N, ...)                                                 \
  N
#define PP_RSEQ_N()                                                           \
  63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, \
      44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, \
      26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9,  \
      8, 7, 6, 5, 4, 3, 2, 1, 0

/* Some test cases */

TEST(VargMacroTest, PPNargTest) {
  EXPECT_EQ(1, PP_NARG(A));
  EXPECT_EQ(2, PP_NARG(A, B));
  EXPECT_EQ(3, PP_NARG(A, B, C));
  EXPECT_EQ(63, PP_NARG(1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8,
                        9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7,
                        8, 9, 0, 1, 2, 3));
}

// Takes each argument 'x' and generates a sequence of mapping from stringified
// token to addres, i.e. {"x", &x}
#define ADDR_MAP(...) ADDR_MAP_(__VA_ARGS__, ADDR_MAP_FILLER())
#define ADDR_MAP_UNWRAP(...) ADDR_MAP_UNWRAP_(__VA_ARGS__, ADDR_MAP_FILLER())

// One layer of indirection, presumably to unify all the commas?
#define ADDR_MAP_(...) ADDR_MAP_TRUNCATE(__VA_ARGS__)
#define ADDR_MAP_UNWRAP_(...) ADDR_MAP_TRUNCATE_UNWRAP(__VA_ARGS__)

// Truncate the list of arguments to only the first. We support at maximum 100
// arguments.

// python code to generate this table:
//
// buf = []
// for item in ["_{0:02d}".format(x) for x in range(100)]:
//   buf.append(item)
//   if len(buf) > 10:
//     print '    ' + ', '.join(buf) + ',\\'
//     buf = []
//
// buf = []
// for item in ["{{#_{0:02d}, &_{0:02d}}}".format(x) for x in range(100)]:
//   buf.append(item)
//   if len(buf) > 4:
//     print ', '.join(buf) + ',\\'
//     buf = []
// clang-format off
#define ADDR_MAP_TRUNCATE(\
    _00, _01, _02, _03, _04, _05, _06, _07, _08, _09,\
    _10, _11, _12, _13, _14, _15, _16, _17, _18, _19,\
    _20, _21, _22, _23, _24, _25, _26, _27, _28, _29,\
    _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,\
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49,\
    _50, _51, _52, _53, _54, _55, _56, _57, _58, _59,\
    _60, _61, _62, _63, _64, _65, _66, _67, _68, _69,\
    _70, _71, _72, _73, _74, _75, _76, _77, _78, _79,\
    _80, _81, _82, _83, _84, _85, _86, _87, _88, _89,\
    _90, _91, _92, _93, _94, _95, _96, _97, _98, _99,\
    ...) \
{#_00, &_00}, {#_01, &_01}, {#_02, &_02}, {#_03, &_03}, {#_04, &_04},\
{#_05, &_05}, {#_06, &_06}, {#_07, &_07}, {#_08, &_08}, {#_09, &_09},\
{#_10, &_10}, {#_11, &_11}, {#_12, &_12}, {#_13, &_13}, {#_14, &_14},\
{#_15, &_15}, {#_16, &_16}, {#_17, &_17}, {#_18, &_18}, {#_19, &_19},\
{#_20, &_20}, {#_21, &_21}, {#_22, &_22}, {#_23, &_23}, {#_24, &_24},\
{#_25, &_25}, {#_26, &_26}, {#_27, &_27}, {#_28, &_28}, {#_29, &_29},\
{#_30, &_30}, {#_31, &_31}, {#_32, &_32}, {#_33, &_33}, {#_34, &_34},\
{#_35, &_35}, {#_36, &_36}, {#_37, &_37}, {#_38, &_38}, {#_39, &_39},\
{#_40, &_40}, {#_41, &_41}, {#_42, &_42}, {#_43, &_43}, {#_44, &_44},\
{#_45, &_45}, {#_46, &_46}, {#_47, &_47}, {#_48, &_48}, {#_49, &_49},\
{#_50, &_50}, {#_51, &_51}, {#_52, &_52}, {#_53, &_53}, {#_54, &_54},\
{#_55, &_55}, {#_56, &_56}, {#_57, &_57}, {#_58, &_58}, {#_59, &_59},\
{#_60, &_60}, {#_61, &_61}, {#_62, &_62}, {#_63, &_63}, {#_64, &_64},\
{#_65, &_65}, {#_66, &_66}, {#_67, &_67}, {#_68, &_68}, {#_69, &_69},\
{#_70, &_70}, {#_71, &_71}, {#_72, &_72}, {#_73, &_73}, {#_74, &_74},\
{#_75, &_75}, {#_76, &_76}, {#_77, &_77}, {#_78, &_78}, {#_79, &_79},\
{#_80, &_80}, {#_81, &_81}, {#_82, &_82}, {#_83, &_83}, {#_84, &_84},\
{#_85, &_85}, {#_86, &_86}, {#_87, &_87}, {#_88, &_88}, {#_89, &_89},\
{#_90, &_90}, {#_91, &_91}, {#_92, &_92}, {#_93, &_93}, {#_94, &_94},\
{#_95, &_95}, {#_96, &_96}, {#_97, &_97}, {#_98, &_98}, {#_99, &_99}
// clang-format on

// It would be really nice if we didn't need a global object for this. Is there
// any globally defined extern variable in C that we could use?
struct InvalidObject {};
extern InvalidObject kInvalidObj;
InvalidObject kInvalidObj{};

// Provide a bunch of extra initializers to fill out the rest of the map.
#define ADDR_MAP_FILLER()                                              \
  kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj,     \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, \
      kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj, kInvalidObj,

TEST(VargMacroTest, PPMapTest) {
  int a, b, c, d, e, f, g, h, i, j, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
  std::map<std::string, void*> map;

  map = std::map<std::string, void*>{ADDR_MAP(a, b, c)};
  EXPECT_EQ(4, map.size());
  EXPECT_EQ(&a, map["a"]);
  EXPECT_EQ(&b, map["b"]);
  EXPECT_EQ(&c, map["c"]);

  map = std::map<std::string, void*>{ADDR_MAP(d, e, f)};
  EXPECT_EQ(4, map.size());
  EXPECT_EQ(&d, map["d"]);
  EXPECT_EQ(&e, map["e"]);
  EXPECT_EQ(&f, map["f"]);
  EXPECT_EQ(map.end(), map.find("a"));
}
