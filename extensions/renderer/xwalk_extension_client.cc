// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/renderer/xwalk_extension_client.h"

#include <Ecore.h>
#include <unistd.h>
#include <v8.h>
#include <json/json.h>

#include <string>

#include "common/logger.h"
#include "common/profiler.h"
#include "common/string_utils.h"
#include "extensions/common/constants.h"
#include "extensions/common/xwalk_extension_server.h"
#include "extensions/renderer/runtime_ipc_client.h"

namespace extensions {

namespace {
  void* CreateInstanceInMainloop(void* data) {
    const char* extension_name = static_cast<const char*>(data);
    XWalkExtensionServer* server = XWalkExtensionServer::GetInstance();
    std::string instance_id = server->CreateInstance(extension_name);
    return static_cast<void*>(new std::string(instance_id));
  }
}  // namespace

XWalkExtensionClient::XWalkExtensionClient() {
}

XWalkExtensionClient::~XWalkExtensionClient() {
  for (auto it = extension_apis_.begin(); it != extension_apis_.end(); ++it) {
    delete it->second;
  }
  extension_apis_.clear();
}

void XWalkExtensionClient::Initialize() {
  SCOPE_PROFILE();
  if (!extension_apis_.empty()) {
    return;
  }

  XWalkExtensionServer* server = XWalkExtensionServer::GetInstance();
  Json::Value reply = server->GetExtensions();
  for (auto it = reply.begin(); it != reply.end(); ++it) {
    ExtensionCodePoints* codepoint = new ExtensionCodePoints;
    Json::Value entry_points = (*it)["entry_points"];
    for (auto ep = entry_points.begin(); ep != entry_points.end(); ++ep) {
      codepoint->entry_points.push_back((*ep).asString());
    }
    std::string name = (*it)["name"].asString();
    extension_apis_[name] = codepoint;
  }
}

std::string XWalkExtensionClient::CreateInstance(
    v8::Handle<v8::Context> context,
    const std::string& extension_name, InstanceHandler* handler) {
  void* ret = ecore_main_loop_thread_safe_call_sync(
      CreateInstanceInMainloop,
      static_cast<void*>(const_cast<char*>(extension_name.data())));
  std::string* sp = static_cast<std::string*>(ret);
  std::string instance_id = *sp;
  delete sp;

  handlers_[instance_id] = handler;
  return instance_id;
}

void XWalkExtensionClient::DestroyInstance(
    v8::Handle<v8::Context> context, const std::string& instance_id) {
  auto it = handlers_.find(instance_id);
  if (it == handlers_.end()) {
    LOGGER(WARN) << "Failed to destory invalid instance id: " << instance_id;
    return;
  }
  RuntimeIPCClient* ipc = RuntimeIPCClient::GetInstance();
  ipc->SendMessage(context, kMethodDestroyInstance, instance_id, "");

  handlers_.erase(it);
}

void XWalkExtensionClient::PostMessageToNative(
    v8::Handle<v8::Context> context,
    const std::string& instance_id, const std::string& msg) {
  RuntimeIPCClient* ipc = RuntimeIPCClient::GetInstance();
  ipc->SendMessage(context, kMethodPostMessage, instance_id, msg);
}

std::string XWalkExtensionClient::SendSyncMessageToNative(
    v8::Handle<v8::Context> context,
    const std::string& instance_id, const std::string& msg) {
  RuntimeIPCClient* ipc = RuntimeIPCClient::GetInstance();
  std::string reply =
      ipc->SendSyncMessage(context, kMethodSendSyncMessage, instance_id, msg);
  return reply;
}

std::string XWalkExtensionClient::GetAPIScript(
    v8::Handle<v8::Context> context,
    const std::string& extension_name) {
  XWalkExtensionServer* server = XWalkExtensionServer::GetInstance();
  return server->GetAPIScript(extension_name);
}

void XWalkExtensionClient::OnReceivedIPCMessage(
    const std::string& instance_id, const std::string& msg) {
  auto it = handlers_.find(instance_id);
  if (it == handlers_.end()) {
    LOGGER(WARN) << "Failed to post the message. Invalid instance id.";
    return;
  }

  if (!it->second)
    return;

  it->second->HandleMessageFromNative(msg);
}
void XWalkExtensionClient::LoadUserExtensions(const std::string app_path) {
  XWalkExtensionServer* server = XWalkExtensionServer::GetInstance();
  server->LoadUserExtensions(app_path);
}

}  // namespace extensions
