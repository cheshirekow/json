// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#pragma once

struct TestA {
  struct {
    int a = 1;
    double b = 3.14;
    float e = 1.2;
    int f = 3;
  } foo;

  struct {
    int c = 2;
    float d = 3.2f;
  } bar;

  struct Boz {
    int a = 1;
    float b = 2.0;
  } boz[2];
};

struct TestB {
  int8_t a = 1;
  int16_t b = 2;
  int32_t c = 3;
  int64_t d = 4;
  uint8_t e = 5;
  uint16_t f = 6;
  uint32_t g = 8;
  float h = 9.0f;
  double i = 10.0;
  bool j = true;
  int8_t k[4] = {1, 2, 3, 4};
  char l[10] = "hello";

  struct {
    int8_t a = 1;
    int8_t b = 2;
  } xbar;

  struct XBaz {
    int8_t a = 1;
    int8_t b = 2;
  } xbaz[3];
};

struct TestC {
  int32_t a = 1;
  int32_t b = 2;

  struct C {
    float d = 3.0f;
    float e = 4.0f;

    struct F {
      uint32_t g = 5;
    } f;
  } c;
};
