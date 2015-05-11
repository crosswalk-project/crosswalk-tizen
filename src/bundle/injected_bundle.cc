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

extern "C" void DynamicSetWidgetInfo(int /*widget_id*/) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicSetWidgetInfo !!";
}

extern "C" void DynamicPluginStartSession(int /*widget_id*/,
                                          v8::Handle<v8::Context> context,
                                          int /*routing_handle*/,
                                          double /*scale*/,
                                          const char* uuid,
                                          const char* /*theme*/,
                                          const char* base_url) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicPluginStartSession !!";
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
    int /*widget_id*/, v8::Handle<v8::Context> context) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicPluginStopSession !!";

  wrt::ExtensionRendererController& controller =
      wrt::ExtensionRendererController::GetInstance();
  controller.WillReleaseScriptContext(context);
}

extern "C" void DynamicUrlParsing(
    std::string* old_url, std::string* new_url, int /*widget_id*/) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicUrlParsing !!";
  *new_url = *old_url;
}

extern "C" void DynamicDatabaseAttach(int /*attach*/) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicDatabaseAttach !!";
}

extern "C" void DynamicOnIPCMessage(const Ewk_IPC_Wrt_Message_Data& /*data*/) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicOnIPCMessage !!";
}

extern "C" void DynamicPreloading() {
  LOGGER(DEBUG) << "InjectedBundle::DynamicPreloading !!";
}
