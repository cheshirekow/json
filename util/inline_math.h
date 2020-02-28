// Copyright (C) 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#pragma once

template <typename T>
int signum(T val) {
  return (T(0) < val) - (val < T(0));
}
