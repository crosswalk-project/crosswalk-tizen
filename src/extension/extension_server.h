// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

  bool Start();
  bool Start(const StringVector& paths);

 private:
  void RegisterExtension(const std::string& path);
  void RegisterSystemExtensions();
  bool RegisterSymbols(Extension* extension);

  void AddRuntimeVariable(const std::string& key, const std::string& value);
  void GetRuntimeVariable(const char* key, char* value, size_t value_len);
  void ClearRuntimeVariables();

  void NotifyEPCreatedToRuntime();
  void HandleDBusMethod(const std::string& method_name,
                        GVariant* parameters,
                        GDBusMethodInvocation* invocation);

  std::string app_uuid_;
  DBusServer dbus_server_;
  DBusClient dbus_runtime_client_;

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
