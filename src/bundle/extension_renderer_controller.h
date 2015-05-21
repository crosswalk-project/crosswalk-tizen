// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_BUNDLE_EXTENSION_RENDERER_CONTROLLER_H_
#define WRT_BUNDLE_EXTENSION_RENDERER_CONTROLLER_H_

#include <v8/v8.h>
#include <string>
#include <memory>

namespace wrt {

class ExtensionClient;

class ExtensionRendererController {
 public:
  static ExtensionRendererController& GetInstance();

  void DidCreateScriptContext(v8::Handle<v8::Context> context);
  void WillReleaseScriptContext(v8::Handle<v8::Context> context);

  bool InitializeExtensions(const std::string& uuid);

 private:
  ExtensionRendererController();
  virtual ~ExtensionRendererController();

 private:
  std::unique_ptr<ExtensionClient> extensions_client_;
};

}  // namespace wrt

#endif  // WRT_BUNDLE_EXTENSION_RENDERER_CONTROLLER_H_
