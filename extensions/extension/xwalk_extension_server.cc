// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/extension/xwalk_extension_server.h"

#include <glib.h>
#include <glob.h>

#include <fstream>
#include <string>
#include <vector>

#include "common/app_db.h"
#include "common/file_utils.h"
#include "common/logger.h"
#include "common/picojson.h"
#include "common/string_utils.h"

#include "extensions/common/constants.h"
#include "extensions/extension/xwalk_extension.h"

namespace extensions {

namespace {

const char kAppDBRuntimeSection[] = "Runtime";

const char kExtensionPrefix[] = "lib";
const char kExtensionSuffix[] = ".so";
const char kExtensionMetadataSuffix[] = ".json";

const char kDBusIntrospectionXML[] =
  "<node>"
  "  <interface name='org.tizen.xwalk.Extension'>"
  "    <method name='GetExtensions'>"
  "      <arg name='extensions' type='a(ssas)' direction='out' />"
  "    </method>"
  "    <method name='GetJavascriptCode'>"
  "      <arg name='extension_name' type='s' direction='in' />"
  "      <arg name='code' type='s' direction='out' />"
  "    </method>"
  "    <method name='CreateInstance'>"
  "      <arg name='extension_name' type='s' direction='in' />"
  "      <arg name='instance_id' type='s' direction='out' />"
  "    </method>"
  "    <method name='DestroyInstance'>"
  "      <arg name='instance_id' type='s' direction='in' />"
  "      <arg name='instance_id' type='s' direction='out' />"
  "    </method>"
  "    <method name='PostMessage'>"
  "      <arg name='instance_id' type='s' direction='in' />"
  "      <arg name='msg' type='s' direction='in' />"
  "    </method>"
  "    <method name='SendSyncMessage'>"
  "      <arg name='instance_id' type='s' direction='in' />"
  "      <arg name='msg' type='s' direction='in' />"
  "      <arg name='reply' type='s' direction='out' />"
  "    </method>"
  "    <signal name='OnMessageToJS'>"
  "      <arg name='instance_id' type='s' />"
  "      <arg name='msg' type='s' />"
  "    </signal>"
  "  </interface>"
  "</node>";

void LoadFrequentlyUsedModules(
    const std::map<std::string, XWalkExtension*>& modules) {
  auto it = modules.find("tizen");
  if (it != modules.end()) {
    it->second->Initialize();
  }
  it = modules.find("xwalk.utils");
  if (it != modules.end()) {
    it->second->Initialize();
  }
}

}  // namespace

XWalkExtensionServer::XWalkExtensionServer(const std::string& appid)
    : appid_(appid) {
}

XWalkExtensionServer::~XWalkExtensionServer() {
}

bool XWalkExtensionServer::Start() {
  return Start(StringVector());
}

bool XWalkExtensionServer::Start(const StringVector& paths) {
  // Register system extensions to support Tizen Device APIs
#ifdef PLUGIN_LAZY_LOADING
  RegisterSystemExtensionsByMetadata();
#else
  RegisterSystemExtensions();
#endif

  // Register user extensions
  for (auto it = paths.begin(); it != paths.end(); ++it) {
    if (common::utils::Exists(*it)) {
      RegisterExtension(*it);
    }
  }

  // Start DBusServer
  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  using std::placeholders::_4;
  dbus_server_.SetIntrospectionXML(kDBusIntrospectionXML);
  dbus_server_.SetMethodCallback(
      kDBusInterfaceNameForExtension,
      std::bind(&XWalkExtensionServer::HandleDBusMethod, this, _1, _2, _3, _4));
  dbus_server_.Start(appid_ + "." + std::string(kDBusNameForExtension));

#ifdef PLUGIN_LAZY_LOADING
  LoadFrequentlyUsedModules(extensions_);
#endif

  return true;
}

void XWalkExtensionServer::RegisterExtension(const std::string& path) {
  XWalkExtension* ext = new XWalkExtension(path, this);
  if (!ext->Initialize() || !RegisterSymbols(ext)) {
    delete ext;
    return;
  }
  extensions_[ext->name()] = ext;
  LOGGER(DEBUG) << ext->name() << " is registered.";
}

void XWalkExtensionServer::RegisterExtension(XWalkExtension* extension) {
  if (!extension->lazy_loading() && !extension->Initialize()) {
    delete extension;
    return;
  }

  if (!RegisterSymbols(extension)) {
    delete extension;
    return;
  }

  extensions_[extension->name()] = extension;
  LOGGER(DEBUG) << extension->name() << " is registered.";
}

void XWalkExtensionServer::RegisterSystemExtensions() {
#ifdef EXTENSION_PATH
  std::string extension_path(EXTENSION_PATH);
#else
  #error EXTENSION_PATH is not set.
#endif
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

void XWalkExtensionServer::RegisterSystemExtensionsByMetadata() {
#ifdef EXTENSION_PATH
  std::string extension_path(EXTENSION_PATH);
#else
  #error EXTENSION_PATH is not set.
#endif
  extension_path.append("/");
  extension_path.append("*");
  extension_path.append(kExtensionMetadataSuffix);

  glob_t glob_result;
  glob(extension_path.c_str(), GLOB_TILDE, NULL, &glob_result);
  for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
    RegisterSystemExtensionsByMetadata(glob_result.gl_pathv[i]);
  }
  if (glob_result.gl_pathc == 0) {
    RegisterSystemExtensions();
  }
}

void XWalkExtensionServer::RegisterSystemExtensionsByMetadata(
    const std::string& metadata_path) {
#ifdef EXTENSION_PATH
  std::string extension_path(EXTENSION_PATH);
#else
  #error EXTENSION_PATH is not set.
#endif

  std::ifstream metafile(metadata_path.c_str());
  if (!metafile.is_open()) {
    LOGGER(ERROR) << "Fail to open the plugin metadata file :" << metadata_path;
    return;
  }

  picojson::value metadata;
  metafile >> metadata;
    if (metadata.is<picojson::array>()) {
    auto& plugins = metadata.get<picojson::array>();
    for (auto plugin = plugins.begin(); plugin != plugins.end(); ++plugin) {
      if (!plugin->is<picojson::object>())
        continue;

      std::string name = plugin->get("name").to_str();
      std::string lib = plugin->get("lib").to_str();
      if (!common::utils::StartsWith(lib, "/")) {
        lib = extension_path + "/" + lib;
      }

      std::vector<std::string> entries;
      auto& entry_points_value = plugin->get("entry_points");
      if (entry_points_value.is<picojson::array>()) {
        auto& entry_points = entry_points_value.get<picojson::array>();
        for (auto entry = entry_points.begin(); entry != entry_points.end();
             ++entry) {
          entries.push_back(entry->to_str());
        }
      }
      XWalkExtension* extension = new XWalkExtension(lib, name, entries, this);
      RegisterExtension(extension);
    }
  } else {
    SLOGE("%s is not plugin metadata", metadata_path.c_str());
  }
  metafile.close();
}


bool XWalkExtensionServer::RegisterSymbols(XWalkExtension* extension) {
  std::string name = extension->name();

  if (extension_symbols_.find(name) != extension_symbols_.end()) {
    LOGGER(WARN) << "Ignoring extension with name already registred. '"
                 << name << "'";
    return false;
  }

  XWalkExtension::StringVector entry_points = extension->entry_points();
  for (auto it = entry_points.begin(); it != entry_points.end(); ++it) {
    if (extension_symbols_.find(*it) != extension_symbols_.end()) {
      LOGGER(WARN) << "Ignoring extension with entry_point already registred. '"
                   << (*it) << "'";
      return false;
    }
  }

  for (auto it = entry_points.begin(); it != entry_points.end(); ++it) {
    extension_symbols_.insert(*it);
  }

  extension_symbols_.insert(name);

  return true;
}

void XWalkExtensionServer::GetRuntimeVariable(const char* key, char* value,
    size_t value_len) {
  common::AppDB* db = common::AppDB::GetInstance();
  std::string ret = db->Get(kAppDBRuntimeSection, key);
  strncpy(value, ret.c_str(), value_len);
}

void XWalkExtensionServer::HandleDBusMethod(GDBusConnection* connection,
                                       const std::string& method_name,
                                       GVariant* parameters,
                                       GDBusMethodInvocation* invocation) {
  if (method_name == kMethodGetExtensions) {
    OnGetExtensions(invocation);
  } else if (method_name == kMethodCreateInstance) {
    gchar* extension_name;
    g_variant_get(parameters, "(&s)", &extension_name);
    OnCreateInstance(connection, extension_name, invocation);
  } else if (method_name == kMethodDestroyInstance) {
    gchar* instance_id;
    g_variant_get(parameters, "(&s)", &instance_id);
    OnDestroyInstance(instance_id, invocation);
  } else if (method_name == kMethodSendSyncMessage) {
    gchar* instance_id;
    gchar* msg;
    g_variant_get(parameters, "(&s&s)", &instance_id, &msg);
    OnSendSyncMessage(instance_id, msg, invocation);
  } else if (method_name == kMethodPostMessage) {
    gchar* instance_id;
    gchar* msg;
    g_variant_get(parameters, "(&s&s)", &instance_id, &msg);
    OnPostMessage(instance_id, msg);
  } else if (method_name == kMethodGetJavascriptCode) {
    gchar* extension_name;
    g_variant_get(parameters, "(&s)", &extension_name);
    OnGetJavascriptCode(connection, extension_name, invocation);
  }
}

void XWalkExtensionServer::OnGetExtensions(GDBusMethodInvocation* invocation) {
  GVariantBuilder builder;

  g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);

  // build an array of extensions
  auto it = extensions_.begin();
  for ( ; it != extensions_.end(); ++it) {
    XWalkExtension* ext = it->second;
    // open container for extension
    g_variant_builder_open(&builder, G_VARIANT_TYPE("(ssas)"));
    g_variant_builder_add(&builder, "s", ext->name().c_str());
    g_variant_builder_add(&builder, "s", ext->javascript_api().c_str());

    // open container for entry_point
    g_variant_builder_open(&builder, G_VARIANT_TYPE("as"));
    auto it_entry = ext->entry_points().begin();
    for ( ; it_entry != ext->entry_points().end(); ++it_entry) {
      g_variant_builder_add(&builder, "s", (*it_entry).c_str());
    }
    // close container('as') for entry_point
    g_variant_builder_close(&builder);
    // close container('(ssas)') for extension
    g_variant_builder_close(&builder);
  }

  GVariant* reply = NULL;
  if (extensions_.size() == 0) {
    reply = g_variant_new_array(G_VARIANT_TYPE("(ssas)"), NULL, 0);
  } else {
    reply = g_variant_builder_end(&builder);
  }

  g_dbus_method_invocation_return_value(
      invocation, g_variant_new_tuple(&reply, 1));
}

void XWalkExtensionServer::OnCreateInstance(
    GDBusConnection* connection, const std::string& extension_name,
    GDBusMethodInvocation* invocation) {
  std::string instance_id = common::utils::GenerateUUID();

  // find extension with given the extension name
  auto it = extensions_.find(extension_name);
  if (it == extensions_.end()) {
    LOGGER(ERROR) << "Failed to find extension '" << extension_name << "'";
    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
        "Not found extension %s", extension_name.c_str());
    return;
  }

  // create instance
  XWalkExtensionInstance* instance = it->second->CreateInstance();
  if (!instance) {
    LOGGER(ERROR) << "Failed to create instance of extension '"
                  << extension_name << "'";
    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
        "Failed to create instance of extension %s", extension_name.c_str());
    return;
  }

  // set callbacks
  using std::placeholders::_1;
  instance->SetPostMessageCallback(
      std::bind(&XWalkExtensionServer::PostMessageToJSCallback,
                this, connection, instance_id, _1));

  instances_[instance_id] = instance;
  g_dbus_method_invocation_return_value(
      invocation, g_variant_new("(s)", instance_id.c_str()));
}

void XWalkExtensionServer::OnDestroyInstance(
    const std::string& instance_id, GDBusMethodInvocation* invocation) {
  // find instance with the given instance id
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOGGER(ERROR) << "Failed to find instance '" << instance_id << "'";
    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
        "Not found instance %s", instance_id.c_str());
    return;
  }

  // destroy the instance
  XWalkExtensionInstance* instance = it->second;
  delete instance;

  instances_.erase(it);

  g_dbus_method_invocation_return_value(
      invocation, g_variant_new("(s)", instance_id.c_str()));
}

void XWalkExtensionServer::OnSendSyncMessage(
    const std::string& instance_id, const std::string& msg,
    GDBusMethodInvocation* invocation) {
  // find instance with the given instance id
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOGGER(ERROR) << "Failed to find instance '" << instance_id << "'";
    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
        "Not found instance %s", instance_id.c_str());
    return;
  }

  XWalkExtensionInstance* instance = it->second;

  using std::placeholders::_1;
  instance->SetSendSyncReplyCallback(
      std::bind(&XWalkExtensionServer::SyncReplyCallback,
                this, _1, invocation));

  instance->HandleSyncMessage(msg);
}

// async
void XWalkExtensionServer::OnPostMessage(
    const std::string& instance_id, const std::string& msg) {
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOGGER(ERROR) << "Failed to find instance '" << instance_id << "'";
    return;
  }

  XWalkExtensionInstance* instance = it->second;
  instance->HandleMessage(msg);
}

void XWalkExtensionServer::OnGetJavascriptCode(GDBusConnection* connection,
                        const std::string& extension_name,
                        GDBusMethodInvocation* invocation) {
  // find extension with given the extension name
  auto it = extensions_.find(extension_name);
  if (it == extensions_.end()) {
    LOGGER(ERROR) << "Failed to find extension '" << extension_name << "'";
    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
        "Not found extension %s", extension_name.c_str());
    return;
  }

  it->second->Initialize();
  g_dbus_method_invocation_return_value(
      invocation, g_variant_new("(s)", it->second->javascript_api().c_str()));
}

void XWalkExtensionServer::SyncReplyCallback(
    const std::string& reply, GDBusMethodInvocation* invocation) {
  g_dbus_method_invocation_return_value(
      invocation, g_variant_new("(s)", reply.c_str()));
}

void XWalkExtensionServer::PostMessageToJSCallback(
    GDBusConnection* connection, const std::string& instance_id,
    const std::string& msg) {
  if (!connection || g_dbus_connection_is_closed(connection)) {
    LOGGER(ERROR) << "Client connection is closed already.";
    return;
  }

  dbus_server_.SendSignal(connection,
                          kDBusInterfaceNameForExtension,
                          kSignalOnMessageToJS,
                          g_variant_new("(ss)",
                                        instance_id.c_str(),
                                        msg.c_str()));
}

}  // namespace extensions
