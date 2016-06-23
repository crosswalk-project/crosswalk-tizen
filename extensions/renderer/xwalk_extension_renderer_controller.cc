// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/renderer/xwalk_extension_renderer_controller.h"

#include <Ecore.h>
#include <v8.h>
#include <string>
#include <utility>

#include "common/logger.h"
#include "common/profiler.h"
#include "extensions/renderer/object_tools_module.h"
#include "extensions/renderer/widget_module.h"
#include "extensions/renderer/xwalk_extension_client.h"
#include "extensions/renderer/xwalk_extension_module.h"
#include "extensions/renderer/xwalk_module_system.h"
#include "extensions/renderer/xwalk_v8tools_module.h"
#include "extensions/renderer/runtime_ipc_client.h"

namespace extensions {

namespace {

void CreateExtensionModules(XWalkExtensionClient* client,
                            XWalkModuleSystem* module_system) {
  const XWalkExtensionClient::ExtensionAPIMap& extensions =
      client->extension_apis();

  for (auto it = extensions.begin(); it != extensions.end(); ++it) {
    XWalkExtensionClient::ExtensionCodePoints* codepoint = it->second;
    std::unique_ptr<XWalkExtensionModule> module(
        new XWalkExtensionModule(client, module_system,
                                 it->first, codepoint->api));
    module_system->RegisterExtensionModule(std::move(module),
                                           codepoint->entry_points);
  }
}

}  // namespace

XWalkExtensionRendererController&
XWalkExtensionRendererController::GetInstance() {
  static XWalkExtensionRendererController instance;
  return instance;
}

XWalkExtensionRendererController::XWalkExtensionRendererController()
    : extensions_client_(new XWalkExtensionClient()) {
}

XWalkExtensionRendererController::~XWalkExtensionRendererController() {
}

void XWalkExtensionRendererController::DidCreateScriptContext(
    v8::Handle<v8::Context> context) {
  SCOPE_PROFILE();
  XWalkModuleSystem* module_system = new XWalkModuleSystem(context);
  XWalkModuleSystem::SetModuleSystemInContext(
      std::unique_ptr<XWalkModuleSystem>(module_system), context);
  module_system->RegisterNativeModule(
        "v8tools",
        std::unique_ptr<XWalkNativeModule>(new XWalkV8ToolsModule));
  module_system->RegisterNativeModule(
        "WidgetModule",
        std::unique_ptr<XWalkNativeModule>(new WidgetModule));
  module_system->RegisterNativeModule(
        "objecttools",
        std::unique_ptr<XWalkNativeModule>(new ObjectToolsModule));

  extensions_client_->Initialize();
  CreateExtensionModules(extensions_client_.get(), module_system);

  module_system->Initialize();
}

void XWalkExtensionRendererController::WillReleaseScriptContext(
    v8::Handle<v8::Context> context) {
  v8::Context::Scope contextScope(context);
  XWalkModuleSystem::ResetModuleSystemFromContext(context);
}

void XWalkExtensionRendererController::OnReceivedIPCMessage(
    const Ewk_IPC_Wrt_Message_Data* data) {

  Eina_Stringshare* type = ewk_ipc_wrt_message_data_type_get(data);

#define TYPE_BEGIN(x) (!strncmp(type, x, strlen(x)))
  if (TYPE_BEGIN("xwalk://"))  {
    Eina_Stringshare* id = ewk_ipc_wrt_message_data_id_get(data);
    Eina_Stringshare* msg = ewk_ipc_wrt_message_data_value_get(data);
    extensions_client_->OnReceivedIPCMessage(id, msg);
    eina_stringshare_del(id);
    eina_stringshare_del(msg);
  } else {
    RuntimeIPCClient* ipc = RuntimeIPCClient::GetInstance();
    ipc->HandleMessageFromRuntime(data);
  }
#undef TYPE_BEGIN

  eina_stringshare_del(type);
}

void XWalkExtensionRendererController::InitializeExtensionClient() {
  extensions_client_->Initialize();
}
void XWalkExtensionRendererController::LoadUserExtensions(
  const std::string app_path) {
  extensions_client_->LoadUserExtensions(app_path);
}

}  // namespace extensions
