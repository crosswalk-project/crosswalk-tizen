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

#include <Ecore.h>
#include <ewk_chromium.h>
#include <unistd.h>
#include <v8.h>

#include <memory>
#include <string>

#include "common/application_data.h"
#include "common/locale_manager.h"
#include "common/logger.h"
#include "common/profiler.h"
#include "common/resource_manager.h"
#include "common/string_utils.h"
#include "extensions/renderer/runtime_ipc_client.h"
#include "extensions/renderer/widget_module.h"
#include "extensions/renderer/xwalk_extension_renderer_controller.h"
#include "extensions/renderer/xwalk_module_system.h"

namespace runtime {
class BundleGlobalData {
 public :
  static BundleGlobalData* GetInstance() {
    static BundleGlobalData instance;
    return &instance;
  }
  void Initialize(const std::string& app_id) {
    app_data_.reset(new common::ApplicationData(app_id));
    app_data_->LoadManifestData();
    locale_manager_.reset(new common::LocaleManager);
    locale_manager_->EnableAutoUpdate(true);
    if (app_data_->widget_info() != NULL &&
        !app_data_->widget_info()->default_locale().empty()) {
      locale_manager_->SetDefaultLocale(
          app_data_->widget_info()->default_locale());
    }
    resource_manager_.reset(new common::ResourceManager(app_data_.get(),
                            locale_manager_.get()));
    resource_manager_->set_base_resource_path(
        app_data_->application_path());

    auto widgetdb = extensions::WidgetPreferenceDB::GetInstance();
    widgetdb->Initialize(app_data_.get(),
                         locale_manager_.get());
  }

  common::ResourceManager* resource_manager() {
    return resource_manager_.get();
  }

 private:
  BundleGlobalData() {}
  ~BundleGlobalData() {}
  std::unique_ptr<common::ResourceManager> resource_manager_;
  std::unique_ptr<common::LocaleManager> locale_manager_;
  std::unique_ptr<common::ApplicationData> app_data_;
};
}  //  namespace runtime

extern "C" void DynamicSetWidgetInfo(const char* tizen_id) {
  SCOPE_PROFILE();
  LOGGER(DEBUG) << "InjectedBundle::DynamicSetWidgetInfo !!" << tizen_id;
  ecore_init();
  runtime::BundleGlobalData::GetInstance()->Initialize(tizen_id);

  STEP_PROFILE_START("Initialize XWalkExtensionRendererController");
  extensions::XWalkExtensionRendererController& controller =
      extensions::XWalkExtensionRendererController::GetInstance();
  controller.InitializeExtensions(tizen_id);
  STEP_PROFILE_END("Initialize XWalkExtensionRendererController");
}

extern "C" void DynamicPluginStartSession(const char* tizen_id,
                                          v8::Handle<v8::Context> context,
                                          int routing_handle,
                                          double /*scale*/,
                                          const char* /*bundle*/,
                                          const char* /*theme*/,
                                          const char* base_url) {
  SCOPE_PROFILE();

  // Initialize context's aligned pointer in embedder data with null
  extensions::XWalkModuleSystem::SetModuleSystemInContext(
      std::unique_ptr<extensions::XWalkModuleSystem>(), context);

  LOGGER(DEBUG) << "InjectedBundle::DynamicPluginStartSession !!" << tizen_id;
  if (base_url == NULL || common::utils::StartsWith(base_url, "http")) {
    LOGGER(ERROR) << "External url not allowed plugin loading.";
    return;
  }

  STEP_PROFILE_START("Initialize RuntimeIPCClient");
  // Initialize RuntimeIPCClient
  extensions::RuntimeIPCClient* rc =
      extensions::RuntimeIPCClient::GetInstance();
  rc->SetRoutingId(context, routing_handle);
  STEP_PROFILE_END("Initialize RuntimeIPCClient");

  extensions::XWalkExtensionRendererController& controller =
      extensions::XWalkExtensionRendererController::GetInstance();
  controller.DidCreateScriptContext(context);
}

extern "C" void DynamicPluginStopSession(
    const char* tizen_id, v8::Handle<v8::Context> context) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicPluginStopSession !!" << tizen_id;

  extensions::XWalkExtensionRendererController& controller =
      extensions::XWalkExtensionRendererController::GetInstance();
  controller.WillReleaseScriptContext(context);
}

extern "C" void DynamicUrlParsing(
    std::string* old_url, std::string* new_url, const char* /*tizen_id*/) {
  auto res_manager =
      runtime::BundleGlobalData::GetInstance()->resource_manager();
  if (res_manager == NULL) {
    LOGGER(ERROR) << "Widget Info was not set, Resource Manager is NULL";
    *new_url = *old_url;
    return;
  }
  // Check Access control
  if (!res_manager->AllowedResource(*old_url)) {
    // deined resource
    *new_url = "about:blank";
    return;
  }
  // convert to localized path
  if (common::utils::StartsWith(*old_url, "file:/") ||
      common::utils::StartsWith(*old_url, "app:/")) {
    *new_url = res_manager->GetLocalizedPath(*old_url);
  } else {
    *new_url = *old_url;
  }
  // check encryption
  if (res_manager->IsEncrypted(*new_url)) {
    *new_url = res_manager->DecryptResource(*new_url);
  }
}

extern "C" void DynamicDatabaseAttach(int /*attach*/) {
  // LOGGER(DEBUG) << "InjectedBundle::DynamicDatabaseAttach !!";
}

extern "C" void DynamicOnIPCMessage(const Ewk_IPC_Wrt_Message_Data& data) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicOnIPCMessage !!";
  extensions::RuntimeIPCClient* rc =
      extensions::RuntimeIPCClient::GetInstance();
  rc->HandleMessageFromRuntime(&data);
}

extern "C" void DynamicPreloading() {
  // LOGGER(DEBUG) << "InjectedBundle::DynamicPreloading !!";
}
