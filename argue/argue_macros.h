#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

// ----------------------------------------------------------------------------
// Begin: Ugly Macro Implementation Details
// ----------------------------------------------------------------------------

// NOTE(josh): inspired by https://stackoverflow.com/a/11640759/141023

// Defer concat of two tokens through one layer of macro indirection so that
// arguments are expanded before they are concatenated.
// See https://gcc.gnu.org/onlinedocs/gcc-2.95.3/cpp_1.html#SEC26
#define ARGUE_PRIMITIVE_CAT(a, b) a##b
#define ARGUE_CAT(a, b) ARGUE_PRIMITIVE_CAT(a, b)

#define ARGUE_8dest_ dest_

#define ARGUE_7metavar_ metavar_
#define ARGUE_7dest_ metavar_ = "", ARGUE_8dest_

#define ARGUE_6help_ help_
#define ARGUE_6metavar_ help_ = "", ARGUE_7metavar_
#define ARGUE_6dest_ help_ = "", ARGUE_7dest_

#define ARGUE_5required_ required_
#define ARGUE_5help_ required_ = false, ARGUE_6help_
#define ARGUE_5metavar_ required_ = false, ARGUE_6metavar_
#define ARGUE_5dest_ required_ = false, ARGUE_6dest_

#define ARGUE_4choices_ choices_
#define ARGUE_4required_ choices_ = {}, ARGUE_5required_
#define ARGUE_4help_ choices_ = {}, ARGUE_5help_
#define ARGUE_4metavar_ choices_ = {}, ARGUE_5metavar_
#define ARGUE_4dest_ choices_ = {}, ARGUE_5dest_

#define ARGUE_3default_ default_
#define ARGUE_3choices_ default_ = argue::NoneType(), ARGUE_4choices_
#define ARGUE_3required_ default_ = argue::NoneType(), ARGUE_4required_
#define ARGUE_3help_ default_ = argue::NoneType(), ARGUE_4help_
#define ARGUE_3metavar_ default_ = argue::NoneType(), ARGUE_4metavar_
#define ARGUE_3dest_ default_ = argue::NoneType(), ARGUE_4dest_

#define ARGUE_2const_ const_
#define ARGUE_2default_ const_ = argue::NoneType(), ARGUE_3default_
#define ARGUE_2choices_ const_ = argue::NoneType(), ARGUE_3choices_
#define ARGUE_2required_ const_ = argue::NoneType(), ARGUE_3required_
#define ARGUE_2help_ const_ = argue::NoneType(), ARGUE_3help_
#define ARGUE_2metavar_ const_ = argue::NoneType(), ARGUE_3metavar_
#define ARGUE_2dest_ const_ = argue::NoneType(), ARGUE_3dest_

#define ARGUE_1nargs_ nargs_
#define ARGUE_1const_ nargs_ = argue::EXACTLY_ONE, ARGUE_2const_
#define ARGUE_1default_ nargs_ = argue::EXACTLY_ONE, ARGUE_2default_
#define ARGUE_1choices_ nargs_ = argue::EXACTLY_ONE, ARGUE_2choices_
#define ARGUE_1required_ nargs_ = argue::EXACTLY_ONE, ARGUE_2required_
#define ARGUE_1help_ nargs_ = argue::EXACTLY_ONE, ARGUE_2help_
#define ARGUE_1metavar_ nargs_ = argue::EXACTLY_ONE, ARGUE_2metavar_
#define ARGUE_1dest_ nargs_ = argue::EXACTLY_ONE, ARGUE_2dest_

#define ARGUE_0action_ action_
#define ARGUE_0nargs_ action_ = "store", ARGUE_1nargs_
#define ARGUE_0const_ action_ = "store", ARGUE_1const_
#define ARGUE_0default_ action_ = "store", ARGUE_1default_
#define ARGUE_0choices_ action_ = "store", ARGUE_1choices_
#define ARGUE_0required_ action_ = "store", ARGUE_1required_
#define ARGUE_0help_ action_ = "store", ARGUE_1help_
#define ARGUE_0metavar_ action_ = "store", ARGUE_1metavar_
#define ARGUE_0dest_ action_ = "store", ARGUE_1dest_

#define ARGUE_SHIM8_(arg0, ...) arg0
#define ARGUE_SHIM7_(arg0, arg1, ...) \
  arg0, ARGUE_SHIM8(ARGUE_CAT(ARGUE_7, arg1), __VA_ARGS__)
#define ARGUE_SHIM6_(arg0, arg1, ...) \
  arg0, ARGUE_SHIM7(ARGUE_CAT(ARGUE_6, arg1), __VA_ARGS__)
#define ARGUE_SHIM5_(arg0, arg1, ...) \
  arg0, ARGUE_SHIM6(ARGUE_CAT(ARGUE_5, arg1), __VA_ARGS__)
#define ARGUE_SHIM4_(arg0, arg1, ...) \
  arg0, ARGUE_SHIM5(ARGUE_CAT(ARGUE_4, arg1), __VA_ARGS__)
#define ARGUE_SHIM3_(arg0, arg1, ...) \
  arg0, ARGUE_SHIM4(ARGUE_CAT(ARGUE_3, arg1), __VA_ARGS__)
#define ARGUE_SHIM2_(arg0, arg1, ...) \
  arg0, ARGUE_SHIM3(ARGUE_CAT(ARGUE_2, arg1), __VA_ARGS__)
#define ARGUE_SHIM1_(arg0, arg1, ...) \
  arg0, ARGUE_SHIM2(ARGUE_CAT(ARGUE_1, arg1), __VA_ARGS__)
#define ARGUE_SHIM0_(arg0, ...) \
  ARGUE_SHIM1(ARGUE_CAT(ARGUE_0, arg0), __VA_ARGS__)

#define ARGUE_SHIM8(...) ARGUE_SHIM8_(__VA_ARGS__)
#define ARGUE_SHIM7(...) ARGUE_SHIM7_(__VA_ARGS__)
#define ARGUE_SHIM6(...) ARGUE_SHIM6_(__VA_ARGS__)
#define ARGUE_SHIM5(...) ARGUE_SHIM5_(__VA_ARGS__)
#define ARGUE_SHIM4(...) ARGUE_SHIM4_(__VA_ARGS__)
#define ARGUE_SHIM3(...) ARGUE_SHIM3_(__VA_ARGS__)
#define ARGUE_SHIM2(...) ARGUE_SHIM2_(__VA_ARGS__)
#define ARGUE_SHIM1(...) ARGUE_SHIM1_(__VA_ARGS__)
#define ARGUE_SHIM0(...) ARGUE_SHIM0_(__VA_ARGS__)

#define ADD_DOTS_(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
  .arg0, .arg1, .arg2, .arg3, .arg4, .arg5, .arg6, .arg7
#define ADD_DOTS(...) ADD_DOTS_(__VA_ARGS__)

#define ARGUE_SHIM(...) ADD_DOTS(ARGUE_SHIM0(__VA_ARGS__, dest_ = nullptr))

// #define ARGUE_KWARG_DEFAULTS \
//   action_ = "store", \
//   nargs_ = argue::EXACTLY_ONE, \
//   const_ = argue::NoneType(), \
//   default_ = argue::NoneType(), \
//   choices_ = {}, \
//   required_ = false, \
//   help_ = "", \
//   metavar_ = "", \
//   dest_ = nullptr
