// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>
#include <v8.h>
#include <ewk_ipc_message.h>
#include <string>

#include "common/logger.h"
#include "common/string_utils.h"
#include "bundle/extension_renderer_controller.h"

extern "C" void DynamicSetWidgetInfo(const char* tizen_id) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicSetWidgetInfo !!" << tizen_id;
}

extern "C" void DynamicPluginStartSession(const char* tizen_id,
                                          v8::Handle<v8::Context> context,
                                          int /*routing_handle*/,
                                          double /*scale*/,
                                          const char* uuid,
                                          const char* /*theme*/,
                                          const char* base_url) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicPluginStartSession !!" << tizen_id;
  if (base_url == NULL || wrt::utils::StartsWith(base_url, "http")) {
    LOGGER(ERROR) << "External url not allowed plugin loading.";
    return;
  }

  wrt::ExtensionRendererController& controller =
      wrt::ExtensionRendererController::GetInstance();
  controller.InitializeExtensions(uuid);
  controller.DidCreateScriptContext(context);
}

extern "C" void DynamicPluginStopSession(
    const char* tizen_id, v8::Handle<v8::Context> context) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicPluginStopSession !!" << tizen_id;

  wrt::ExtensionRendererController& controller =
      wrt::ExtensionRendererController::GetInstance();
  controller.WillReleaseScriptContext(context);
}

extern "C" void DynamicUrlParsing(
    std::string* old_url, std::string* new_url, const char* tizen_id) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicUrlParsing !!" << tizen_id;
  *new_url = *old_url;
}

extern "C" void DynamicDatabaseAttach(const char* tizen_id) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicDatabaseAttach !!" << tizen_id;
}

extern "C" void DynamicOnIPCMessage(const Ewk_IPC_Wrt_Message_Data& /*data*/) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicOnIPCMessage !!";
}

extern "C" void DynamicPreloading() {
  LOGGER(DEBUG) << "InjectedBundle::DynamicPreloading !!";
}
