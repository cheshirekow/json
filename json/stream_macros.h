#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

// ----------------------------------------------------------------------------
// Begin: Ugly Macro Implementation Details
// ----------------------------------------------------------------------------

/// Takes each argument 'x' and generates a sequence of pairs of stringified
/// token to address, i.e.
/// JSON_ADDRMAP(x, y, z) -> "x", &x, "y", &y, "z", &z
#define JSON_ADDRMAP(...) JSON_ADDRMAP_(__VA_ARGS__, JSON_ADDRFILL())

/// One layer of indirection, presumably to unify all the commas?
#define JSON_ADDRMAP_(...) JSON_ADDRMAP_TRUNC(__VA_ARGS__)

// clang-format off
/// Truncate the list of arguments to only the first. We support at maximum 100
/// arguments.
#define JSON_ADDRMAP_TRUNC(\
    _00, _01, _02, _03, _04, _05, _06, _07, _08, _09, \
    _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
    _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, \
    _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, \
    _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, \
    _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, \
    _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, \
    _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, \
    _90, _91, _92, _93, _94, _95, _96, _97, _98, _99, \
    ...) \
#_00, &_00, #_01, &_01, #_02, &_02, #_03, &_03, #_04, &_04, \
#_05, &_05, #_06, &_06, #_07, &_07, #_08, &_08, #_09, &_09, \
#_10, &_10, #_11, &_11, #_12, &_12, #_13, &_13, #_14, &_14, \
#_15, &_15, #_16, &_16, #_17, &_17, #_18, &_18, #_19, &_19, \
#_20, &_20, #_21, &_21, #_22, &_22, #_23, &_23, #_24, &_24, \
#_25, &_25, #_26, &_26, #_27, &_27, #_28, &_28, #_29, &_29, \
#_30, &_30, #_31, &_31, #_32, &_32, #_33, &_33, #_34, &_34, \
#_35, &_35, #_36, &_36, #_37, &_37, #_38, &_38, #_39, &_39, \
#_40, &_40, #_41, &_41, #_42, &_42, #_43, &_43, #_44, &_44, \
#_45, &_45, #_46, &_46, #_47, &_47, #_48, &_48, #_49, &_49, \
#_50, &_50, #_51, &_51, #_52, &_52, #_53, &_53, #_54, &_54, \
#_55, &_55, #_56, &_56, #_57, &_57, #_58, &_58, #_59, &_59, \
#_60, &_60, #_61, &_61, #_62, &_62, #_63, &_63, #_64, &_64, \
#_65, &_65, #_66, &_66, #_67, &_67, #_68, &_68, #_69, &_69, \
#_70, &_70, #_71, &_71, #_72, &_72, #_73, &_73, #_74, &_74, \
#_75, &_75, #_76, &_76, #_77, &_77, #_78, &_78, #_79, &_79, \
#_80, &_80, #_81, &_81, #_82, &_82, #_83, &_83, #_84, &_84, \
#_85, &_85, #_86, &_86, #_87, &_87, #_88, &_88, #_89, &_89, \
#_90, &_90, #_91, &_91, #_92, &_92, #_93, &_93, #_94, &_94, \
#_95, &_95, #_96, &_96, #_97, &_97, #_98, &_98, #_99, &_99
// clang-format on

/// Provide a bunch of extra initializers to fill out the rest of the map.
#define JSON_ADDRFILL()                                                        \
  kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad, kMacroPad,        \
      kMacroPad, kMacroPad, kMacroPad

/// Implement parse-map construction for the given fields of the struct
#define JSON_DEFPARSER(...)                                 \
  void GetParseMap(json::stream::ParseMap* pmap) {          \
    json::stream::FillMap(pmap, JSON_ADDRMAP(__VA_ARGS__)); \
  }

/// Implement emit-map construction for the given fields of the struct
#define JSON_DEFEMITTER(...)                                \
  void GetEmitMap(json::stream::EmitMap* emap) const {      \
    json::stream::FillMap(emap, JSON_ADDRMAP(__VA_ARGS__)); \
  }

// ----------------------------------------------------------------------------
// End: Ugly Macro Implementation Details
// ----------------------------------------------------------------------------

/// Implement parse-map and emit-map getters for JSON-stream API
/**
 * Usage:
 *
 * ```
 * struct MyObject {
 *   int a;
 *   float b;
 *   JSON_STREAM(a,b);
 * };
 * ```
 * Adds two member functions:
 * ```
 * void GetParseMap(json::ParseMap* pmap);
 * void GetEmitMap(json::EmitMap* emap);
 * ```
 */
#define JSON_STREAM(...)       \
  JSON_DEFPARSER(__VA_ARGS__); \
  JSON_DEFEMITTER(__VA_ARGS__);
