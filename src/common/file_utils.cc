// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "file_utils.h"

#include <unistd.h>
#include <libgen.h>
#include <string>

namespace wrt {
namespace utils {

bool Exist(const std::string& path) {
  return (access(path.c_str(), F_OK) != -1);
}

std::string BaseName(const std::string& path) {
  char* p = basename(const_cast<char*>(path.c_str()));
  return std::string(p);
}

std::string DirName(const std::string& path) {
  char* p = dirname(const_cast<char*>(path.c_str()));
  return std::string(p);
}


} // namespace utils
} // namespace wrt
