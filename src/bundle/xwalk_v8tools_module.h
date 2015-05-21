// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_BUNDLE_XWALK_V8TOOLS_MODULE_H_
#define WRT_BUNDLE_XWALK_V8TOOLS_MODULE_H_

#include "bundle/module_system.h"

namespace wrt {

// This module provides extra JS functions that help writing JS API code for
// extensions, for example: allowing setting a read-only property of an object.
class XWalkV8ToolsModule : public NativeModule {
 public:
  XWalkV8ToolsModule();
  ~XWalkV8ToolsModule() override;

 private:
  v8::Handle<v8::Object> NewInstance() override;

  v8::Persistent<v8::ObjectTemplate> object_template_;
};

}  // namespace wrt

#endif  // WRT_BUNDLE_XWALK_V8TOOLS_MODULE_H_