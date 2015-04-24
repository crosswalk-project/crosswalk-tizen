// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/string_utils.h"

#include <time.h>
#include <math.h>
#include <uuid/uuid.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace wrt {
namespace utils {

std::string GenerateUUID() {
  char tmp[37];
  uuid_t uuid;
  uuid_generate(uuid);
  uuid_unparse(uuid, tmp);
  return std::string(tmp);
}

bool StartsWith(const std::string& str, const std::string& sub) {
  if (sub.size() > str.size()) return false;
  return std::equal(sub.begin(), sub.end(), str.begin());
}

bool EndsWith(const std::string& str, const std::string& sub) {
  if (sub.size() > str.size()) return false;
  return std::equal(sub.rbegin(), sub.rend(), str.rbegin());
}

std::string ReplaceAll(const std::string& replace,
                       const std::string& from, const std::string& to) {
  std::string str = replace;
  int pos = str.find(from);
  while (pos != std::string::npos) {
    str.replace(pos, from.length(), to);
    pos = str.find(from, pos+to.length());
  }
  return str;
}

std::string GetCurrentMilliSeconds() {
  std::ostringstream ss;
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  ss << spec.tv_sec << "." <<
     std::setw(3) << std::setfill('0') << (round(spec.tv_nsec / 1.0e6));
  return ss.str();
}

}  // namespace utils
}  // namespace wrt
