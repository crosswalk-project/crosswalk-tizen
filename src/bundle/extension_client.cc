// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bundle/extension_client.h"

#include <glib.h>
#include <gio/gio.h>
#include <string>

#include "common/logger.h"
#include "common/constants.h"
#include "common/profiler.h"
#include "common/string_utils.h"

namespace wrt {

ExtensionClient::ExtensionClient() {
}

ExtensionClient::~ExtensionClient() {
  auto it = extension_apis_.begin();
  for ( ; it != extension_apis_.end(); ++it) {
    delete it->second;
  }
}

std::string ExtensionClient::CreateInstance(
    const std::string& extension_name, InstanceHandler* handler) {
  GVariant* value = dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodCreateInstance,
      g_variant_new("(s)", extension_name.c_str()),
      G_VARIANT_TYPE("(s)"));

  if (!value) {
    LoggerE("Failed to create instance for extension %s",
            extension_name.c_str());
    return std::string();
  }

  gchar* instance_id;
  g_variant_get(value, "(&s)", &instance_id);

  std::string ret(instance_id);
  handlers_[ret] = handler;

  g_variant_unref(value);
  return ret;
}

void ExtensionClient::DestroyInstance(const std::string& instance_id) {
  GVariant* value = dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodDestroyInstance,
      g_variant_new("(s)", instance_id.c_str()),
      G_VARIANT_TYPE("(s)"));

  if (!value) {
    LoggerE("Failed to destroy instance %s", instance_id.c_str());
    return;
  }

  auto it = handlers_.find(instance_id);
  if (it != handlers_.end()) {
    handlers_.erase(it);
  }

  g_variant_unref(value);
}

void ExtensionClient::PostMessageToNative(
    const std::string& instance_id, const std::string& msg) {
  dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodPostMessage,
      g_variant_new("(ss)", instance_id.c_str(), msg.c_str()),
      NULL);
}

std::string ExtensionClient::SendSyncMessageToNative(
    const std::string& instance_id, const std::string& msg) {
  GVariant* value = dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodSendSyncMessage,
      g_variant_new("(ss)", instance_id.c_str(), msg.c_str()),
      G_VARIANT_TYPE("(s)"));

  if (!value) {
    LoggerE("Failed to send synchronous message to ExtensionServer.");
    return std::string();
  }

  gchar* reply;
  g_variant_get(value, "(&s)", &reply);

  std::string ret(reply);
  g_variant_unref(value);

  return ret;
}

void ExtensionClient::Initialize(const std::string& uuid) {
  // Connect to DBusServer for ExtensionServer
  if (dbus_extension_client_.ConnectByName(
          uuid + "." + std::string(kDBusNameForExtension))) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    dbus_extension_client_.SetSignalCallback(
        kDBusInterfaceNameForExtension,
        std::bind(&ExtensionClient::HandleSignal, this, _1, _2));
  } else {
    LoggerE("Failed to connect to the dbus server for Extension.");
    return;
  }

  // get extensions from ExtensionServer
  GVariant* value = dbus_extension_client_.Call(
      kDBusInterfaceNameForExtension, kMethodGetExtensions,
      NULL,
      G_VARIANT_TYPE("(a(ssas))"));

  if (!value) {
    LoggerE("Failed to get extension list from ExtensionServer.");
    return;
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
}

void ExtensionClient::HandleSignal(
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

}  // namespace wrt
