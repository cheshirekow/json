// g++ -I . -E -c argue/test/macro_test.cc
#include "argue/argue_macros.h"

#define DEMO_dd d

#define DEMO_cc c
#define DEMO_cd c = 3, DEMO_dd

#define DEMO_bb b
#define DEMO_bc b = 2, DEMO_cc
#define DEMO_bd b = 2, DEMO_cd

#define DEMO_aa a
#define DEMO_ab a = 1, DEMO_bb
#define DEMO_ac a = 1, DEMO_bc
#define DEMO_ad a = 1, DEMO_bd

#define DEMO_default DEMO_ad = 4

#define DEMO_SHIM4_(arg0, ...) arg0
#define DEMO_SHIM3_(arg0, arg1, ...) \
  arg0, DEMO_SHIM4(ARGUE_CAT(DEMO_d, arg1), __VA_ARGS__)
#define DEMO_SHIM2_(arg0, arg1, ...) \
  arg0, DEMO_SHIM3(ARGUE_CAT(DEMO_c, arg1), __VA_ARGS__)
#define DEMO_SHIM1_(arg0, arg1, ...) \
  arg0, DEMO_SHIM2(ARGUE_CAT(DEMO_b, arg1), __VA_ARGS__)
#define DEMO_SHIM0_(arg0, ...) DEMO_SHIM1(ARGUE_CAT(DEMO_a, arg0), __VA_ARGS__)

#define DEMO_SHIM0(...) DEMO_SHIM0_(__VA_ARGS__)
#define DEMO_SHIM1(...) DEMO_SHIM1_(__VA_ARGS__)
#define DEMO_SHIM2(...) DEMO_SHIM2_(__VA_ARGS__)
#define DEMO_SHIM3(...) DEMO_SHIM3_(__VA_ARGS__)
#define DEMO_SHIM4(...) DEMO_SHIM4_(__VA_ARGS__)

#define DEMO_SHIM(...) DEMO_SHIM0(__VA_ARGS__, d = 4)

DEMO_SHIM(a = 9, b = 8, c = 7, d = 6)  // a=9, b=8, c=7, d=6
DEMO_SHIM(b = 8, c = 7, d = 6)         // a=1, b=8, c=7, d=6
DEMO_SHIM(c = 7, d = 6)                // a=1, b=2, c=7, d=6
DEMO_SHIM(d = 6)                       // a=1, b=2, c=3, d=6

DEMO_SHIM(a = 9)                       // a = 9, b = 2, c = 3, d = 4
DEMO_SHIM(a = 9, b = 8)                // a = 9, b = 8, c = 3, d = 4
DEMO_SHIM(a = 9, b = 8, c = 7)         // a = 9, b = 8, c = 7, d = 4
DEMO_SHIM(a = 9, b = 8, c = 7, d = 6)  // a = 9, b = 8, c = 7, d = 6

DEMO_SHIM(b = 8)  // a = 1, b = 8, c = 3, d = 4
DEMO_SHIM(c = 7)  // a = 1, b = 2, c = 7, d = 4
DEMO_SHIM(d = 6)  // a = 1, b = 2, c = 3, d = 6

DEMO_SHIM(b = 8, d = 6)  // a = 1, b = 8, c = 3, d = 6

ARGUE_SHIM(action_ = "store")
// action_ = "store",
// nargs_ = argue::EXACTLY_ONE,
// const_ = argue::NoneType(),
// default_ = argue::NoneType(),
// choices_ = {},
// required_ = false,
// help_ = "",
// metavar_ = ""
