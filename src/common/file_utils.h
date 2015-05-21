// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_FILE_UTILS_H_
#define WRT_COMMON_FILE_UTILS_H_

#include <string>

namespace wrt {
namespace utils {

bool Exists(const std::string& path);

std::string BaseName(const std::string& path);

std::string DirName(const std::string& path);

std::string SchemeName(const std::string& uri);


}  // namespace utils
}  // namespace wrt

#endif  // WRT_COMMON_FILE_UTILS_H_
