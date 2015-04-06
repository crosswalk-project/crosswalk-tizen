// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_EXTENSION_EXTENSION_SERVER_H_
#define WRT_EXTENSION_EXTENSION_SERVER_H_

#include <string>
#include <vector>

#include "extensin/extension_manager.h"

namespace wrt {

class ExtensionServer {
 public:
  ExtensionServer();
  virtual ~ExtensionServer();

  // 'path' can indicate a file or a directory.
  // if the 'path' indicate a directory, ExtensionServer will load all of
  // extensions in the directory.
  void AddExtensionPath(const std::string& path);
 private:
  std::vector<std::string> extension_paths_;
  ExtensionManager extension_manager_;
};

}  // namespace wrt

#endif  // WRT_EXTENSION_EXTENSION_SERVER_H_
