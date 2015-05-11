// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bundle/extension_renderer_controller.h"

#include <v8/v8.h>
#include <string>
#include <utility>

#include "common/logger.h"
#include "bundle/extension_client.h"
#include "bundle/extension_module.h"
#include "bundle/module_system.h"

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
