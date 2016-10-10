// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_XWALK_EXTENSION_SERVER_H_
#define XWALK_EXTENSIONS_XWALK_EXTENSION_SERVER_H_

#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <json/json.h>

#include <string>
#include <map>

#include "extensions/common/xwalk_extension_manager.h"
#include "extensions/common/xwalk_extension_instance.h"

namespace extensions {

class XWalkExtensionServer {
 public:
  static XWalkExtensionServer* GetInstance();

  void SetupIPC(Ewk_Context* ewk_context);
  void Preload();
  Json::Value GetExtensions();
  std::string GetAPIScript(const std::string& extension_name);
  std::string CreateInstance(const std::string& extension_name);

  void HandleIPCMessage(Ewk_IPC_Wrt_Message_Data* data);

  void Shutdown();
  void LoadUserExtensions(const std::string app_path);

 private:
  XWalkExtensionServer();
  virtual ~XWalkExtensionServer();

  void HandleGetExtensions(Ewk_IPC_Wrt_Message_Data* data);
  void HandleCreateInstance(Ewk_IPC_Wrt_Message_Data* data);
  void HandleDestroyInstance(Ewk_IPC_Wrt_Message_Data* data);
  void HandlePostMessageToNative(Ewk_IPC_Wrt_Message_Data* data);
  void HandleSendSyncMessageToNative(Ewk_IPC_Wrt_Message_Data* data);
  void HandleGetAPIScript(Ewk_IPC_Wrt_Message_Data* data);

  typedef std::map<std::string, XWalkExtensionInstance*> InstanceMap;

  Ewk_Context* ewk_context_;

  XWalkExtensionManager manager_;

  InstanceMap instances_;
};

}  // namespace extensions

#endif  // XWALK_EXTENSIONS_XWALK_EXTENSION_SERVER_H_
