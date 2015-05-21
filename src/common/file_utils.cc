// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/file_utils.h"

#include <unistd.h>
#include <libgen.h>
#include <string>

namespace wrt {
namespace utils {

bool Exists(const std::string& path) {
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

std::string SchemeName(const std::string& uri) {
  size_t pos = uri.find(":");
  if (pos != std::string::npos && pos < uri.length()) {
    return std::string(uri.substr(0, pos));
  } else {
    return std::string();
  }
}

}  // namespace utils
}  // namespace wrt
