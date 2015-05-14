// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_BUNDLE_EXTENSION_MODULE_H_
#define WRT_BUNDLE_EXTENSION_MODULE_H_

#include <v8/v8.h>

#include <memory>
#include <string>

#include "bundle/extension_client.h"

namespace wrt {

class ModuleSystem;

// Responsible for running the JS code of a Extension. This includes
// creating and exposing an 'extension' object for the execution context of
// the extension JS code.
//
// We'll create one ExtensionModule per extension/frame pair, so
// there'll be a set of different modules per v8::Context.
class ExtensionModule : public ExtensionClient::InstanceHandler {
 public:
  ExtensionModule(ExtensionClient* client,
                  ModuleSystem* module_system,
                  const std::string& extension_name,
                  const std::string& extension_code);
  virtual ~ExtensionModule();

  // TODO(cmarcelo): Make this return a v8::Handle<v8::Object>, and
  // let the module system set it to the appropriated object.
  void LoadExtensionCode(v8::Handle<v8::Context> context);

  std::string extension_name() const { return extension_name_; }

 private:
  // ExtensionClient::InstanceHandler implementation.
  virtual void HandleMessageFromNative(const std::string& msg);

  // Callbacks for JS functions available in 'extension' object.
  static void PostMessageCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void SendSyncMessageCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void SetMessageListenerCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void SendRuntimeMessageCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void SendRuntimeSyncMessageCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  static void SendRuntimeAsyncMessageCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info);

  static ExtensionModule* GetExtensionModule(
      const v8::FunctionCallbackInfo<v8::Value>& info);

  // Template for the 'extension' object exposed to the extension JS code.
  v8::Persistent<v8::ObjectTemplate> object_template_;

  // This JS object contains a pointer back to the ExtensionModule, it is
  // set as data for the function callbacks.
  v8::Persistent<v8::Object> function_data_;

  // Function to be called when the extension sends a message to its JS code.
  // This value is registered by using 'extension.setMessageListener()'.
  v8::Persistent<v8::Function> message_listener_;

  std::string extension_name_;
  std::string extension_code_;

  ExtensionClient* client_;
  ModuleSystem* module_system_;
  std::string instance_id_;
};

}  // namespace wrt

#endif  // WRT_BUNDLE_EXTENSION_MODULE_H_
