// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_EXTENSION_EXTENSION_SERVER_H_
#define WRT_EXTENSION_EXTENSION_SERVER_H_

#include <ewk_ipc_message.h>

#include <string>
#include <set>
#include <map>
#include <vector>

#include "extension/extension.h"

class Ewk_Context;

namespace wrt {

class ExtensionServer : public Extension::ExtensionDelegate {
 public:
  typedef std::vector<std::string> StringVector;

  explicit ExtensionServer(Ewk_Context* ewk_context);
  virtual ~ExtensionServer();

  void Start();
  void Start(const StringVector& paths);

  void HandleWrtMessage(Ewk_IPC_Wrt_Message_Data* message);

 private:
  void RegisterExtension(const std::string& path);
  void RegisterSystemExtensions();
  bool RegisterSymbols(Extension* extension);

  void AddRuntimeVariable(const std::string& key, const std::string& value);
  void GetRuntimeVariable(const char* key, char* value, size_t value_len);
  void ClearRuntimeVariables();

  void SendWrtMessage(const std::string& type);
  void SendWrtMessage(const std::string& type, const std::string& id,
                      const std::string& ref_id, const std::string& value);

  void OnGetExtensions(const std::string& id);
  void OnCreateInstance(const std::string& instance_id,
                        const std::string& extension_name);
  void OnDestroyInstance(const std::string& instance_id);
  void OnSendSyncMessageToNative(const std::string& msg_id,
                                 const std::string& instance_id,
                                 const std::string& msg_body);
  void OnPostMessageToNative(const std::string& instance_id,
                             const std::string& msg_body);

  void OnPostMessageToJS(const std::string& instance_id,
                         const std::string& msg);
  void OnSendSyncReplyToJS(const std::string& instance_id,
                           const std::string& msg);

  Ewk_Context* ewk_context_;

  typedef std::set<std::string> StringSet;
  StringSet extension_symbols_;

  typedef std::map<std::string, std::string> StringMap;
  StringMap runtime_variables_;

  typedef std::map<std::string, Extension*> ExtensionMap;
  ExtensionMap extensions_;

  typedef std::map<std::string, ExtensionInstance*> InstanceMap;
  InstanceMap instances_;
};

}  // namespace wrt

#endif  // WRT_EXTENSION_EXTENSION_SERVER_H_
