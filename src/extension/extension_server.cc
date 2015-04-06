// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extension/extension_server.h"

ExtensionServer::ExtensionServer() {
}

ExtensionServer::~ExtensionServer() {
}

void ExtensionServer::AddExtensionPath(const std::string& path) {
  extension_paths_.push_back(path);
}
