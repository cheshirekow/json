// Copyright (C) 2014 Josh Bialkowski (josh.bialkowski@gmail.com)
/**
 *  @file
 *  @date   Aug 4, 2013
 *  @author Josh Bialkowski (josh.bialkowski@gmail.com)
 */

#pragma once
#include <ctime>

timespec operator+(const timespec& a, const timespec& b);
timespec operator-(const timespec& a, const timespec& b);

inline bool operator==(const timespec& a, const timespec& b) {
  return a.tv_sec == b.tv_sec && a.tv_nsec == b.tv_nsec;
}

inline bool operator!=(const timespec& a, const timespec& b) {
  return a.tv_sec != b.tv_sec && a.tv_nsec != b.tv_nsec;
}

inline bool operator<(const timespec& a, const timespec& b) {
  if (a.tv_sec == b.tv_sec) {
    return a.tv_nsec < b.tv_nsec;
  } else {
    return a.tv_sec < b.tv_sec;
  }
}

inline bool operator>(const timespec& a, const timespec& b) {
  if (a.tv_sec == b.tv_sec) {
    return a.tv_nsec > b.tv_nsec;
  } else {
    return a.tv_sec > b.tv_sec;
  }
}

inline bool operator<=(const timespec& a, const timespec& b) {
  if (a.tv_sec == b.tv_sec) {
    return a.tv_nsec <= b.tv_nsec;
  } else {
    return a.tv_sec <= b.tv_sec;
  }
}

inline bool operator>=(const timespec& a, const timespec& b) {
  if (a.tv_sec == b.tv_sec) {
    return a.tv_nsec >= b.tv_nsec;
  } else {
    return a.tv_sec >= b.tv_sec;
  }
}
