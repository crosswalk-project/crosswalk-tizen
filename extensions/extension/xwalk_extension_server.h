// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_XWALK_EXTENSION_SERVER_H_
#define XWALK_EXTENSIONS_XWALK_EXTENSION_SERVER_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "common/dbus_client.h"
#include "common/dbus_server.h"
#include "extensions/extension/xwalk_extension.h"

namespace extensions {

class XWalkExtensionServer : public XWalkExtension::XWalkExtensionDelegate {
 public:
  typedef std::vector<std::string> StringVector;

  explicit XWalkExtensionServer(const std::string& appid);
  virtual ~XWalkExtensionServer();

  bool Start();
  bool Start(const StringVector& paths);

 private:
  void RegisterExtension(const std::string& path);
  void RegisterExtension(XWalkExtension* extension);
  void RegisterSystemExtensions();
  void RegisterSystemExtensionsByMetadata();
  void RegisterSystemExtensionsByMetadata(const std::string& metadata_path);
  bool RegisterSymbols(XWalkExtension* extension);

  void GetRuntimeVariable(const char* key, char* value, size_t value_len);

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

  std::string appid_;
  common::DBusServer dbus_server_;
  common::DBusClient dbus_application_client_;

  typedef std::set<std::string> StringSet;
  StringSet extension_symbols_;

  typedef std::map<std::string, XWalkExtension*> ExtensionMap;
  ExtensionMap extensions_;

  typedef std::map<std::string, XWalkExtensionInstance*> InstanceMap;
  InstanceMap instances_;
};

}  // namespace extensions

#endif  // XWALK_EXTENSIONS_XWALK_EXTENSION_SERVER_H_
