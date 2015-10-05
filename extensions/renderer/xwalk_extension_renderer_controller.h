// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDERER_CONTROLLER_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDERER_CONTROLLER_H_

#include <v8/v8.h>
#include <memory>
#include <string>

namespace extensions {

class XWalkExtensionClient;

class XWalkExtensionRendererController {
 public:
  static XWalkExtensionRendererController& GetInstance();

  void DidCreateScriptContext(v8::Handle<v8::Context> context);
  void WillReleaseScriptContext(v8::Handle<v8::Context> context);

  bool InitializeExtensions(const std::string& appid);

 private:
  XWalkExtensionRendererController();
  virtual ~XWalkExtensionRendererController();

 private:
  std::unique_ptr<XWalkExtensionClient> extensions_client_;
};

}  // namespace extensions

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_RENDERER_CONTROLLER_H_
