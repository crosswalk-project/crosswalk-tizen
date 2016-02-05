// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "extensions/common/xwalk_extension.h"
#include "extensions/common/xwalk_extension_instance.h"
#include "extensions/common/xwalk_extension_manager.h"

namespace extensions {

class XWalkExtensionClient {
 public:
  typedef std::map<std::string, XWalkExtension*> ExtensionMap;
  typedef std::map<std::string, XWalkExtensionInstance*> InstanceMap;

  struct InstanceHandler {
    virtual void HandleMessageFromNative(const std::string& msg) = 0;
   protected:
    ~InstanceHandler() {}
  };

  XWalkExtensionClient();
  virtual ~XWalkExtensionClient();

  void Initialize();

  XWalkExtension* GetExtension(const std::string& extension_name);
  ExtensionMap GetExtensions();

  std::string CreateInstance(const std::string& extension_name,
                             InstanceHandler* handler);
  void DestroyInstance(const std::string& instance_id);

  void PostMessageToNative(const std::string& instance_id,
                           const std::string& msg);
  std::string SendSyncMessageToNative(const std::string& instance_id,
                                      const std::string& msg);

 private:
  XWalkExtensionManager manager_;
  InstanceMap instances_;
};

}  // namespace extensions

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_EXTENSION_CLIENT_H_
