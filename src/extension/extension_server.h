/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef WRT_EXTENSION_EXTENSION_SERVER_H_
#define WRT_EXTENSION_EXTENSION_SERVER_H_

#include <string>
#include <set>
#include <map>
#include <vector>

#include "common/dbus_server.h"
#include "common/dbus_client.h"
#include "extension/extension.h"

class Ewk_Context;

namespace wrt {

class ExtensionServer : public Extension::ExtensionDelegate {
 public:
  typedef std::vector<std::string> StringVector;

  explicit ExtensionServer(const std::string& appid);
  virtual ~ExtensionServer();

  static bool StartExtensionProcess();

  bool Start();
  bool Start(const StringVector& paths);

 private:
  void RegisterExtension(const std::string& path);
  void RegisterExtension(Extension* extension);
  void RegisterSystemExtensions();
  void RegisterSystemExtensionsByMetadata();
  bool RegisterSymbols(Extension* extension);

  void GetRuntimeVariable(const char* key, char* value, size_t value_len);

  void NotifyEPCreatedToApplication();

  void HandleDBusMethod(GDBusConnection* connection,
                        const std::string& method_name,
                        GVariant* parameters,
                        GDBusMethodInvocation* invocation);

  void OnGetExtensions(GDBusMethodInvocation* invocation);
  void OnCreateInstance(GDBusConnection* connection,
                        const std::string& extension_name,
                        GDBusMethodInvocation* invocation);
  void OnDestroyInstance(const std::string& instance_id,
                         GDBusMethodInvocation* invocation);
  void OnSendSyncMessage(const std::string& instance_id,
                         const std::string& msg,
                         GDBusMethodInvocation* invocation);
  void OnPostMessage(const std::string& instance_id,
                     const std::string& msg);

  void SyncReplyCallback(const std::string& reply,
                         GDBusMethodInvocation* invocation);

  void PostMessageToJSCallback(GDBusConnection* connection,
                               const std::string& instance_id,
                               const std::string& msg);
  void OnGetJavascriptCode(GDBusConnection* connection,
                        const std::string& extension_name,
                        GDBusMethodInvocation* invocation);

  std::string app_uuid_;
  DBusServer dbus_server_;
  DBusClient dbus_application_client_;

  typedef std::set<std::string> StringSet;
  StringSet extension_symbols_;

  typedef std::map<std::string, Extension*> ExtensionMap;
  ExtensionMap extensions_;

  typedef std::map<std::string, ExtensionInstance*> InstanceMap;
  InstanceMap instances_;
};

}  // namespace wrt

#endif  // WRT_EXTENSION_EXTENSION_SERVER_H_
