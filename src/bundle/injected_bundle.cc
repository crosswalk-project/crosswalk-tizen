// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>
#include <v8.h>
#include <ewk_ipc_message.h>
#include <Ecore.h>
#include <string>
#include <memory>

#include "common/logger.h"
#include "common/string_utils.h"
#include "common/application_data.h"
#include "common/resource_manager.h"
#include "common/locale_manager.h"
#include "bundle/runtime_ipc_client.h"
#include "bundle/extension_renderer_controller.h"

namespace wrt {
class BundleGlobalData {
 public :
  static BundleGlobalData* GetInstance() {
    static BundleGlobalData instance;
    return &instance;
  }
  void Initialize(const std::string& app_id) {
    app_data_.reset(new ApplicationData(app_id));
    app_data_->LoadManifestData();
    locale_manager_.reset(new LocaleManager);
    locale_manager_->EnableAutoUpdate(true);
    if (app_data_->widget_info() != NULL &&
        !app_data_->widget_info()->default_locale().empty()) {
      locale_manager_->SetDefaultLocale(
          app_data_->widget_info()->default_locale());
    }
    resource_manager_.reset(new ResourceManager(app_data_.get(),
                                                locale_manager_.get()));
    resource_manager_->set_base_resource_path(
        app_data_->application_path());
  }

  ResourceManager* resource_manager() {
    return resource_manager_.get();
  }

 private:
  BundleGlobalData() {}
  ~BundleGlobalData() {}
  std::unique_ptr<ResourceManager> resource_manager_;
  std::unique_ptr<LocaleManager> locale_manager_;
  std::unique_ptr<ApplicationData> app_data_;
};
}  //  namespace wrt

extern "C" void DynamicSetWidgetInfo(const char* tizen_id) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicSetWidgetInfo !!" << tizen_id;
  ecore_init();
  wrt::BundleGlobalData::GetInstance()->Initialize(tizen_id);
}

extern "C" void DynamicPluginStartSession(const char* tizen_id,
                                          v8::Handle<v8::Context> context,
                                          int routing_handle,
                                          double /*scale*/,
                                          const char* uuid,
                                          const char* /*theme*/,
                                          const char* base_url) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicPluginStartSession !!" << tizen_id;
  if (base_url == NULL || wrt::utils::StartsWith(base_url, "http")) {
    LOGGER(ERROR) << "External url not allowed plugin loading.";
    return;
  }

  // Initialize RuntimeIPCClient
  wrt::RuntimeIPCClient* rc = wrt::RuntimeIPCClient::GetInstance();
  rc->set_routing_id(routing_handle);

  // Initialize ExtensionRendererController
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
    std::string* old_url, std::string* new_url, const char* /*tizen_id*/) {
  auto res_manager = wrt::BundleGlobalData::GetInstance()->resource_manager();
  if (res_manager == NULL) {
    LOGGER(ERROR) << "Widget Info was not set, Resource Manager is NULL";
    *new_url = *old_url;
    return;
  }
  *new_url = res_manager->GetLocalizedPath(*old_url);
}

extern "C" void DynamicDatabaseAttach(int /*attach*/) {
  // LOGGER(DEBUG) << "InjectedBundle::DynamicDatabaseAttach !!";
}

extern "C" void DynamicOnIPCMessage(const Ewk_IPC_Wrt_Message_Data& data) {
  LOGGER(DEBUG) << "InjectedBundle::DynamicOnIPCMessage !!";
  wrt::RuntimeIPCClient* rc = wrt::RuntimeIPCClient::GetInstance();
  rc->HandleMessageFromRuntime(&data);
}

extern "C" void DynamicPreloading() {
  // LOGGER(DEBUG) << "InjectedBundle::DynamicPreloading !!";
}
