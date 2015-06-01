/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "bundle/extension_renderer_controller.h"

#include <v8/v8.h>
#include <string>
#include <utility>

#include "common/logger.h"
#include "bundle/extension_client.h"
#include "bundle/extension_module.h"
#include "bundle/module_system.h"
#include "bundle/xwalk_v8tools_module.h"
#include "bundle/widget_module.h"


namespace wrt {

namespace {

void CreateExtensionModules(ExtensionClient* client,
                            ModuleSystem* module_system) {
  const ExtensionClient::ExtensionAPIMap& extensions =
      client->extension_apis();
  auto it = extensions.begin();
  for (; it != extensions.end(); ++it) {
    ExtensionClient::ExtensionCodePoints* codepoint = it->second;
    if (codepoint->api.empty())
      continue;

    std::unique_ptr<ExtensionModule> module(
        new ExtensionModule(client, module_system, it->first, codepoint->api));
    module_system->RegisterExtensionModule(
        std::move(module), codepoint->entry_points);
  }
}

}  // namespace

ExtensionRendererController& ExtensionRendererController::GetInstance() {
  static ExtensionRendererController instance;
  return instance;
}

ExtensionRendererController::ExtensionRendererController()
    : extensions_client_(new ExtensionClient()) {
}

ExtensionRendererController::~ExtensionRendererController() {
}

void ExtensionRendererController::DidCreateScriptContext(
    v8::Handle<v8::Context> context) {
  ModuleSystem* module_system = new ModuleSystem(context);
  ModuleSystem::SetModuleSystemInContext(
      std::unique_ptr<ModuleSystem>(module_system), context);

  module_system->RegisterNativeModule(
        "v8tools", std::unique_ptr<NativeModule>(new XWalkV8ToolsModule));
  module_system->RegisterNativeModule(
        "WidgetModule", std::unique_ptr<NativeModule>(new WidgetModule));

  CreateExtensionModules(extensions_client_.get(), module_system);
  module_system->Initialize();
}

void ExtensionRendererController::WillReleaseScriptContext(
    v8::Handle<v8::Context> context) {
  v8::Context::Scope contextScope(context);
  ModuleSystem::ResetModuleSystemFromContext(context);
}

bool ExtensionRendererController::InitializeExtensions(
    const std::string& uuid) {
  return extensions_client_->Initialize(uuid);
}

}  // namespace wrt
