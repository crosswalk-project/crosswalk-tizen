// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_STRING_UTILS_H_
#define WRT_COMMON_STRING_UTILS_H_

#include <string>

namespace wrt {
namespace utils {

std::string GenerateUUID();
bool StartsWith(const std::string& str, const std::string& sub);
bool EndsWith(const std::string& str, const std::string& sub);
std::string ReplaceAll(const std::string& replace,
                       const std::string& from, const std::string& to);
std::string GetCurrentMilliSeconds();

}  // namespace utils
}  // namespace wrt

#endif  // WRT_COMMON_STRING_UTILS_H_
