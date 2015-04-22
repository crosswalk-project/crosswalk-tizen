// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extension/extension_server.h"

#include <glob.h>
#include <string>
#include <vector>

#include "common/logger.h"
#include "common/constants.h"
#include "common/file_utils.h"
#include "extension/extension.h"

namespace wrt {

namespace {

const char kDBusIntrospectionXML[] =
  "<node>"
  "  <interface name='org.tizen.wrt.Extension'>"
  "    <method name='GetExtensions'>"
  "      <arg name='extensions' type='a(ssas)' direction='out' />"
  "    </method>"
  "    <method name='CreateInstance'>"
  "      <arg name='extension_name' type='s' direction='in' />"
  "      <arg name='instance_id' type='s' direction='in' />"
  "    </method>"
  "    <method name='DestroyInstance'>"
  "      <arg name='instance_id' type='s' direction='in' />"
  "    </method>"
  "    <method name='CallASync'>"
  "      <arg name='instance_id' type='s' direction='in' />"
  "      <arg name='body' type='s' direction='in' />"
  "    </method>"
  "    <method name='CallSync'>"
  "      <arg name='instance_id' type='s' direction='in' />"
  "      <arg name='body' type='s' direction='in' />"
  "      <arg name='reply' type='s' direction='out' />"
  "    </method>"
  "    <signal name='OnMessageToJS'>"
  "      <arg name='instance_id' type='s' />"
  "      <arg name='body' type='s' />"
  "    </signal>"
  "  </interface>"
  "</node>";

}  // namespace

ExtensionServer::ExtensionServer(const std::string& uuid)
    : app_uuid_(uuid) {
}

ExtensionServer::~ExtensionServer() {
}

bool ExtensionServer::Start() {
  return Start(StringVector());
}

bool ExtensionServer::Start(const StringVector& paths) {
  // Register system extensions to support Tizen Device APIs
  RegisterSystemExtensions();

  // Register user extensions
  for (auto it = paths.begin(); it != paths.end(); ++it) {
    if (utils::Exists(*it)) {
      RegisterExtension(*it);
    }
  }

  // Connect to DBusServer for Runtime
  if (!dbus_runtime_client_.ConnectByName(
          app_uuid_ + "." + std::string(kDBusNameForRuntime))) {
    LoggerE("Failed to connect to the dbus server for Runtime.");
    return false;
  }

  // Start DBusServer
  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  dbus_server_.SetIntrospectionXML(kDBusIntrospectionXML);
  dbus_server_.SetMethodCallback(
      kDBusInterfaceNameForExtension,
      std::bind(&ExtensionServer::HandleDBusMethod, this, _1, _2, _3));
  dbus_server_.Start(app_uuid_ + "." + std::string(kDBusNameForExtension));

  // Send 'ready' signal to Injected Bundle.
  NotifyEPCreatedToRuntime();

  return true;
}

void ExtensionServer::RegisterExtension(const std::string& path) {
  Extension* ext = new Extension(path, this);
  if (!ext->Initialize() || !RegisterSymbols(ext)) {
    delete ext;
  }
  extensions_[ext->name()] = ext;
}

void ExtensionServer::RegisterSystemExtensions() {
  std::string extension_path(kSystemExtensionPath);
  extension_path.append("/");
  extension_path.append(kExtensionPrefix);
  extension_path.append("*");
  extension_path.append(kExtensionSuffix);

  glob_t glob_result;
  glob(extension_path.c_str(), GLOB_TILDE, NULL, &glob_result);
  for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
    RegisterExtension(glob_result.gl_pathv[i]);
  }
}

bool ExtensionServer::RegisterSymbols(Extension* extension) {
  std::string name = extension->name();

  if (extension_symbols_.find(name) != extension_symbols_.end()) {
    LoggerW("Ignoring extension with name already registred. '%s'",
            name.c_str());
    return false;
  }

  Extension::StringVector entry_points = extension->entry_points();
  for (auto it = entry_points.begin(); it != entry_points.end(); ++it) {
    if (extension_symbols_.find(*it) != extension_symbols_.end()) {
      LoggerW("Ignoring extension with entry_point already registred. '%s'",
              (*it).c_str());
      return false;
    }
  }

  for (auto it = entry_points.begin(); it != entry_points.end(); ++it) {
    extension_symbols_.insert(*it);
  }

  extension_symbols_.insert(name);

  return true;
}

void ExtensionServer::AddRuntimeVariable(const std::string& key,
    const std::string& value) {
  runtime_variables_.insert(std::make_pair(key, value));
}

void ExtensionServer::ClearRuntimeVariables() {
  runtime_variables_.clear();
}

void ExtensionServer::GetRuntimeVariable(const char* key, char* value,
    size_t value_len) {
  auto it = runtime_variables_.find(key);
  if (it != runtime_variables_.end()) {
    strncpy(value, it->second.c_str(), value_len);
  }
}

void ExtensionServer::NotifyEPCreatedToRuntime() {
  dbus_runtime_client_.Call(
      kDBusInterfaceNameForRuntime, kMethodNotifyEPCreated,
      g_variant_new("(s)", dbus_server_.GetClientAddress().c_str()),
      NULL);
}

void ExtensionServer::HandleDBusMethod(const std::string& method_name,
                                       GVariant* parameters,
                                       GDBusMethodInvocation* invocation) {
  // TODO(wy80.choi): Handle DBus Method Calls from InjectedBundle
}


}  // namespace wrt
