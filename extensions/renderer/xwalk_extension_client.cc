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
#include "common/string_utils.h"
#include "extensions/common/constants.h"

namespace extensions {

XWalkExtensionClient::XWalkExtensionClient() {
}

XWalkExtensionClient::~XWalkExtensionClient() {
}

void XWalkExtensionClient::Initialize() {
  manager_.LoadExtensions();
}

XWalkExtension* XWalkExtensionClient::GetExtension(
    const std::string& extension_name) {
  // find extension with given the extension name
  ExtensionMap extensions = manager_.extensions();
  auto it = extensions.find(extension_name);
  if (it == extensions.end()) {
    LOGGER(ERROR) << "No such extension '" << extension_name << "'";
    return nullptr;
  }

  return it->second;
}

XWalkExtensionClient::ExtensionMap XWalkExtensionClient::GetExtensions() {
  return manager_.extensions();
}

std::string XWalkExtensionClient::CreateInstance(
    const std::string& extension_name, InstanceHandler* handler) {
  std::string instance_id = common::utils::GenerateUUID();

  // find extension with given the extension name
  ExtensionMap extensions = manager_.extensions();
  auto it = extensions.find(extension_name);
  if (it == extensions.end()) {
    LOGGER(ERROR) << "No such extension '" << extension_name << "'";
    return std::string();
  }

  // create instance
  XWalkExtensionInstance* instance = it->second->CreateInstance();
  if (!instance) {
    LOGGER(ERROR) << "Failed to create instance of extension '"
                  << extension_name << "'";
    return std::string();
  }

  // set callbacks
  using std::placeholders::_1;
  instance->SetPostMessageCallback([handler](const std::string& msg) {
    if (handler) {
      handler->HandleMessageFromNative(msg);
    }
  });

  instances_[instance_id] = instance;
  return instance_id;
}

void XWalkExtensionClient::DestroyInstance(const std::string& instance_id) {
  // find instance with the given instance id
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOGGER(ERROR) << "No such instance '" << instance_id << "'";
    return;
  }

  // destroy the instance
  XWalkExtensionInstance* instance = it->second;
  delete instance;

  instances_.erase(it);
}

void XWalkExtensionClient::PostMessageToNative(
    const std::string& instance_id, const std::string& msg) {
  // find instance with the given instance id
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOGGER(ERROR) << "No such instance '" << instance_id << "'";
    return;
  }

  // Post a message
  XWalkExtensionInstance* instance = it->second;
  instance->HandleMessage(msg);
}

std::string XWalkExtensionClient::SendSyncMessageToNative(
    const std::string& instance_id, const std::string& msg) {
  // find instance with the given instance id
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LOGGER(ERROR) << "No such instance '" << instance_id << "'";
    return std::string();
  }

  // Post a message and receive a reply message
  std::string reply;
  XWalkExtensionInstance* instance = it->second;
  instance->SetSendSyncReplyCallback([&reply](const std::string& msg) {
    reply = msg;
  });
  instance->HandleSyncMessage(msg);
  return reply;
}

}  // namespace extensions
