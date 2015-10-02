// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/renderer/xwalk_extension_client.h"

#include <gio/gio.h>
#include <glib.h>
#include <unistd.h>

#include <string>

#include "common/logger.h"
#include "common/profiler.h"
#include "extensions/common/constants.h"

namespace extensions {

XWalkExtensionClient::XWalkExtensionClient() {
}

XWalkExtensionClient::~XWalkExtensionClient() {
  auto it = extension_apis_.begin();
  for ( ; it != extension_apis_.end(); ++it) {
    delete it->second;
  }
}

std::string XWalkExtensionClient::CreateInstance(
    const std::string& extension_name, InstanceHandler* handler) {
  GVariant* value = dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodCreateInstance,
      g_variant_new("(s)", extension_name.c_str()),
      G_VARIANT_TYPE("(s)"));

  if (!value) {
    LOGGER(ERROR) << "Failed to create instance for extension "
                  << extension_name;
    return std::string();
  }

  gchar* instance_id;
  g_variant_get(value, "(&s)", &instance_id);

  std::string ret(instance_id);
  handlers_[ret] = handler;

  g_variant_unref(value);
  return ret;
}

void XWalkExtensionClient::DestroyInstance(const std::string& instance_id) {
  GVariant* value = dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodDestroyInstance,
      g_variant_new("(s)", instance_id.c_str()),
      G_VARIANT_TYPE("(s)"));

  if (!value) {
    LOGGER(ERROR) << "Failed to destroy instance " << instance_id;
    return;
  }

  auto it = handlers_.find(instance_id);
  if (it != handlers_.end()) {
    handlers_.erase(it);
  }

  g_variant_unref(value);
}

void XWalkExtensionClient::PostMessageToNative(
    const std::string& instance_id, const std::string& msg) {
  dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodPostMessage,
      g_variant_new("(ss)", instance_id.c_str(), msg.c_str()),
      NULL);
}

std::string XWalkExtensionClient::SendSyncMessageToNative(
    const std::string& instance_id, const std::string& msg) {
  GVariant* value = dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodSendSyncMessage,
      g_variant_new("(ss)", instance_id.c_str(), msg.c_str()),
      G_VARIANT_TYPE("(s)"));

  if (!value) {
    LOGGER(ERROR) << "Failed to send synchronous message to ExtensionServer.";
    return std::string();
  }

  gchar* reply;
  g_variant_get(value, "(&s)", &reply);

  std::string ret(reply);
  g_variant_unref(value);

  return ret;
}

bool XWalkExtensionClient::Initialize(const std::string& appid) {
  STEP_PROFILE_START("Connect ExtensionServer");
  // Retry connecting to ExtensionServer
  // ExtensionServer can not be ready at this time yet.
  const int retry_max = 20;
  bool connected = false;
  for (int i=0; i < retry_max; i++) {
    connected = dbus_extension_client_.ConnectByName(
        appid + "." + std::string(kDBusNameForExtension));
    if (connected) break;
    LOGGER(WARN) << "Failed to connect to ExtensionServer. retry "
                 << (i+1) << "/" << retry_max;
    usleep(50*1000);
  }
  STEP_PROFILE_END("Connect ExtensionServer");

  if (!connected) {
    LOGGER(ERROR) << "Failed to connect to the dbus server for Extension.";
    return false;
  }

  // Set signal handler
  using std::placeholders::_1;
  using std::placeholders::_2;
  dbus_extension_client_.SetSignalCallback(
      kDBusInterfaceNameForExtension,
      std::bind(&XWalkExtensionClient::HandleSignal, this, _1, _2));

  // get extensions from ExtensionServer
  GVariant* value = dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodGetExtensions,
      NULL,
      G_VARIANT_TYPE("(a(ssas))"));

  if (!value) {
    LOGGER(ERROR) << "Failed to get extension list from ExtensionServer.";
    return false;
  }

  gchar* name;
  gchar* jsapi;
  gchar* entry_point;
  GVariantIter *it;
  GVariantIter* entry_it;

  g_variant_get(value, "(a(ssas))", &it);
  while (g_variant_iter_loop(it, "(ssas)", &name, &jsapi, &entry_it)) {
    ExtensionCodePoints* code = new ExtensionCodePoints;
    code->api = std::string(jsapi);
    while (g_variant_iter_loop(entry_it, "s", &entry_point)) {
      code->entry_points.push_back(std::string(entry_point));
    }
    extension_apis_.insert(std::make_pair(std::string(name), code));
  }

  g_variant_unref(value);

  return true;
}

void XWalkExtensionClient::HandleSignal(
    const std::string& signal_name, GVariant* parameters) {
  if (signal_name == kSignalOnMessageToJS) {
    gchar* instance_id;
    gchar* msg;
    g_variant_get(parameters, "(&s&s)", &instance_id, &msg);
    auto it = handlers_.find(instance_id);
    if (it != handlers_.end()) {
      InstanceHandler* handler = it->second;
      if (handler) {
        handler->HandleMessageFromNative(msg);
      }
    }
  }
}

std::string XWalkExtensionClient::GetExtensionJavascriptAPICode(
    const std::string& name) {

  auto extension_code_point = extension_apis_.find(name);
  if (extension_code_point == extension_apis_.end())
    return std::string();

  ExtensionCodePoints* code_points = extension_code_point->second;
  if (!code_points->api.empty()) {
    return code_points->api;
  }

  GVariant* value = dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodGetJavascriptCode,
      g_variant_new("(s)", name.c_str()),
      G_VARIANT_TYPE("(s)"));
  gchar* api;
  g_variant_get(value, "(&s)", &api);
  code_points->api = std::string(api);
  g_variant_unref(value);
  return code_points->api;
}

}  // namespace extensions
