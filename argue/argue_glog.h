#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/argue.h"

namespace argue {

// Add glog options (normally exposed through gflags) to the parser
void AddGlogOptions(Parser* parser);

}  // namespace argue
