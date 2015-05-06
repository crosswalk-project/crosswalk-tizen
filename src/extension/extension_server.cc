// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extension/extension_server.h"

#include <glob.h>
#include <glib.h>
#include <glib-unix.h>

#include <string>
#include <vector>

#include "common/logger.h"
#include "common/constants.h"
#include "common/file_utils.h"
#include "common/string_utils.h"
#include "common/command_line.h"
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
  // Connect to DBusServer for Application of Runtime
  if (!dbus_application_client_.ConnectByName(
          app_uuid_ + "." + std::string(kDBusNameForApplication))) {
    LoggerE("Failed to connect to the dbus server for Application.");
    return false;
  }

  // Start DBusServer
  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  using std::placeholders::_4;
  dbus_server_.SetIntrospectionXML(kDBusIntrospectionXML);
  dbus_server_.SetMethodCallback(
      kDBusInterfaceNameForExtension,
      std::bind(&ExtensionServer::HandleDBusMethod, this, _1, _2, _3, _4));
  dbus_server_.Start(app_uuid_ + "." + std::string(kDBusNameForExtension));

  // Register system extensions to support Tizen Device APIs
  RegisterSystemExtensions();

  // Register user extensions
  for (auto it = paths.begin(); it != paths.end(); ++it) {
    if (utils::Exists(*it)) {
      RegisterExtension(*it);
    }
  }

  // Send 'ready' signal to Injected Bundle.
  NotifyEPCreatedToApplication();

  return true;
}

void ExtensionServer::RegisterExtension(const std::string& path) {
  Extension* ext = new Extension(path, this);
  if (!ext->Initialize() || !RegisterSymbols(ext)) {
    delete ext;
    return;
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

void ExtensionServer::GetRuntimeVariable(const char* key, char* value,
    size_t value_len) {
  GVariant* ret = dbus_application_client_.Call(
      kDBusInterfaceNameForApplication, kMethodGetRuntimeVariable,
      g_variant_new("(s)", key), G_VARIANT_TYPE("(s)"));

  if (!ret) {
    LoggerE("Failed to get runtime variable from Application. (%s)", key);
    return;
  }

  gchar* v;
  g_variant_get(ret, "(&s)", &v);
  strncpy(value, v, value_len);

  g_variant_unref(ret);
}

void ExtensionServer::NotifyEPCreatedToApplication() {
  dbus_application_client_.Call(
      kDBusInterfaceNameForApplication, kMethodNotifyEPCreated,
      g_variant_new("(s)", dbus_server_.GetClientAddress().c_str()),
      NULL);
}

void ExtensionServer::HandleDBusMethod(GDBusConnection* connection,
                                       const std::string& method_name,
                                       GVariant* parameters,
                                       GDBusMethodInvocation* invocation) {
  LoggerD("HandleDBusMethod (%s)", method_name.c_str());

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
  }
}

void ExtensionServer::OnGetExtensions(GDBusMethodInvocation* invocation) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);

  // build an array of extensions
  auto it = extensions_.begin();
  for ( ; it != extensions_.end(); ++it) {
    Extension* ext = it->second;
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
  GVariant* reply = g_variant_builder_end(&builder);
  g_dbus_method_invocation_return_value(
      invocation, g_variant_new_tuple(&reply, 1));
}

void ExtensionServer::OnCreateInstance(
    GDBusConnection* connection, const std::string& extension_name,
    GDBusMethodInvocation* invocation) {
  std::string instance_id = utils::GenerateUUID();

  // find extension with given the extension name
  auto it = extensions_.find(extension_name);
  if (it == extensions_.end()) {
    LoggerE("Failed to find extension [%s]", extension_name.c_str());
    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
        "Not found extension %s", extension_name.c_str());
    return;
  }

  // create instance
  ExtensionInstance* instance = it->second->CreateInstance();
  if (!instance) {
    LoggerE("Failed to create instance of extension [%s]",
            extension_name.c_str());
    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
        "Failed to create instance of extension %s", extension_name.c_str());
    return;
  }

  // set callbacks
  using std::placeholders::_1;
  instance->SetSendSyncReplyCallback(
      std::bind(&ExtensionServer::SyncReplyCallback, this, _1, invocation));
  instance->SetPostMessageCallback(
      std::bind(&ExtensionServer::PostMessageToJSCallback,
                this, connection, instance_id, _1));

  instances_[instance_id] = instance;
  g_dbus_method_invocation_return_value(
      invocation, g_variant_new("(s)", instance_id.c_str()));
}

void ExtensionServer::OnDestroyInstance(
    const std::string& instance_id, GDBusMethodInvocation* invocation) {
  // find instance with the given instance id
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LoggerE("Failed to find instance [%s]", instance_id.c_str());
    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
        "Not found instance %s", instance_id.c_str());
    return;
  }

  // destroy the instance
  ExtensionInstance* instance = it->second;
  delete instance;

  instances_.erase(it);

  g_dbus_method_invocation_return_value(
      invocation, g_variant_new("(s)", instance_id.c_str()));
}

void ExtensionServer::OnSendSyncMessage(
    const std::string& instance_id, const std::string& msg,
    GDBusMethodInvocation* invocation) {
  // find instance with the given instance id
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LoggerE("Failed to find instance [%s]", instance_id.c_str());
    g_dbus_method_invocation_return_error(
        invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
        "Not found instance %s", instance_id.c_str());
    return;
  }

  ExtensionInstance* instance = it->second;
  instance->HandleSyncMessage(msg);

  // reponse will be sent by SyncReplyCallback()
}

// async
void ExtensionServer::OnPostMessage(
    const std::string& instance_id, const std::string& msg) {
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LoggerE("Failed to find instance [%s]", instance_id.c_str());
    return;
  }

  ExtensionInstance* instance = it->second;
  instance->HandleMessage(msg);
}

void ExtensionServer::SyncReplyCallback(
    const std::string& reply, GDBusMethodInvocation* invocation) {
  g_dbus_method_invocation_return_value(
      invocation, g_variant_new("(s)", reply.c_str()));
}

void ExtensionServer::PostMessageToJSCallback(
    GDBusConnection* connection, const std::string& instance_id,
    const std::string& msg) {
  if (!connection || g_dbus_connection_is_closed(connection)) {
    LoggerE("Client connection is closed already.");
    return;
  }

  dbus_server_.SendSignal(connection,
                          kDBusInterfaceNameForExtension,
                          kSignalOnMessageToJS,
                          g_variant_new("(ss)",
                                        instance_id.c_str(),
                                        msg.c_str()));
}

// static
bool ExtensionServer::StartExtensionProcess() {
  GMainLoop* loop;

  loop = g_main_loop_new(NULL, FALSE);

  // Register Quit Signal Handlers
  auto quit_callback = [](gpointer data) -> gboolean {
    GMainLoop* loop = reinterpret_cast<GMainLoop*>(data);
    g_main_loop_quit(loop);
    return false;
  };
  g_unix_signal_add(SIGINT, quit_callback, loop);
  g_unix_signal_add(SIGTERM, quit_callback, loop);

  CommandLine* cmd = CommandLine::ForCurrentProcess();

  // TODO(wy80.choi): Receive extension paths for user defined extensions.

  // Receive AppID from arguments.
  if (cmd->arguments().size() < 1) {
    LoggerE("uuid is required.");
    return false;
  }
  std::string uuid = cmd->arguments()[0];

  // Start ExtensionServer
  ExtensionServer server(uuid);
  if (!server.Start()) {
    LoggerE("Failed to start extension server.");
    return false;
  }

  LoggerI("extension process has been started.");

  g_main_loop_run(loop);

  LoggerI("extension process is exiting.");

  g_main_loop_unref(loop);

  return true;
}

}  // namespace wrt
