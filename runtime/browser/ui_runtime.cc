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

#include "runtime/browser/runtime.h"

#include <ewk_chromium.h>
#include <Ecore.h>

#include <memory>
#include <string>
#include <vector>

#include "common/application_data.h"
#include "common/app_control.h"
#include "common/app_db.h"
#include "common/command_line.h"
#include "common/logger.h"
#include "common/profiler.h"
#include "runtime/common/constants.h"
#include "runtime/browser/native_app_window.h"
#include "runtime/browser/notification_window.h"
#include "runtime/browser/preload_manager.h"
#include "runtime/browser/ui_runtime.h"

namespace runtime {

namespace {

const char kCategoryAlwaysOnTop[] = "http://tizen.org/category/always_on_top";

static NativeWindow* CreateNativeWindow(NativeWindow::Type windowType) {
  SCOPE_PROFILE();

  NativeWindow* window;

  switch (windowType) {
    case NativeWindow::Type::NORMAL: {
      auto cached = PreloadManager::GetInstance()->GetCachedNativeWindow();
      window = cached != nullptr ? cached : new NativeAppWindow();
    }
    break;
    case NativeWindow::Type::NOTIFICATION: {
      PreloadManager::GetInstance()->ReleaseCachedNativeWindow();
      NotificationWindow* win = new NotificationWindow();
      window = win;
    }
    break;
  }

  window->Initialize();
  return window;
}

bool HasAlwaysTopCategory(
    const std::vector<std::string>& category_info_list) {
  for (const auto& category : category_info_list) {
      if (strcmp(category.c_str(), kCategoryAlwaysOnTop) == 0)
        return true;
    }
    return false;
}

bool AlwaysTopRequested(common::AppControl* appcontrol) {
  return appcontrol->data("always_on_top") == "true";
}

}  // namespace

UiRuntime::UiRuntime(common::ApplicationData* app_data)
    : native_window_(nullptr),
      application_(nullptr),
      context_(
        ewk_context_new_with_injected_bundle_path(INJECTED_BUNDLE_PATH)),
      app_data_(app_data) {
}

UiRuntime::~UiRuntime() {
  application_.reset();
  native_window_.reset();
  if (context_)
      ewk_context_delete(context_);
}

void UiRuntime::ResetWebApplication(NativeWindow::Type windowType) {
  STEP_PROFILE_START("Runtime ResetWebApplication");

  LOGGER(DEBUG) << "runtime.cc ResetWebApplication() started";

  application_.reset();
  native_window_.reset();

  native_window_.reset(CreateNativeWindow(windowType));
  LOGGER(DEBUG) << "runtime.cc Created native window";
  application_.reset(new WebApplication(native_window_.get(),
                                        app_data_,
                                        context_));
  LOGGER(DEBUG) << "runtime.cc created web application";
  application_->set_terminator([](){ ui_app_exit(); });

  LOGGER(DEBUG) << "runtime.cc ResetWebApplication() finished";

  STEP_PROFILE_END("Runtime ResetWebApplication");
}

bool UiRuntime::OnCreate() {
  STEP_PROFILE_END("ui_app_main -> OnCreate");
  STEP_PROFILE_END("Start -> OnCreate");
  STEP_PROFILE_START("OnCreate -> URL Set");

  common::CommandLine* cmd = common::CommandLine::ForCurrentProcess();
  std::string appid = cmd->GetAppIdFromCommandLine(kRuntimeExecName);

  // Init AppDB for Runtime
  common::AppDB* appdb = common::AppDB::GetInstance();
  appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeName, "xwalk-tizen");
  appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeAppID, appid);
  appdb->Remove(kAppDBRuntimeSection, kAppDBRuntimeBundle);

  ResetWebApplication(NativeWindow::Type::NORMAL);

  setlocale(LC_ALL, "");
  bindtextdomain(kTextDomainRuntime, kTextLocalePath);

  return true;
}

void UiRuntime::OnTerminate() {
  application_.reset();
  native_window_.reset();
}

void UiRuntime::OnPause() {
  if (application_->launched()) {
    application_->Suspend();
  }
}

void UiRuntime::OnResume() {
  if (application_->launched()) {
    application_->Resume();
  }
}

void UiRuntime::OnAppControl(app_control_h app_control) {
  SCOPE_PROFILE();
  std::unique_ptr<common::AppControl>
      appcontrol(new common::AppControl(app_control));
  common::AppDB* appdb = common::AppDB::GetInstance();
  appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeBundle,
             appcontrol->encoded_bundle());

  bool reset_webapp = false;
  NativeWindow::Type type;
  if (AlwaysTopRequested(appcontrol.get()) &&
      HasAlwaysTopCategory(app_data_->category_info_list()->categories)) {
    if (native_window_->type() != NativeWindow::Type::NOTIFICATION) {
      type = NativeWindow::Type::NOTIFICATION;
      reset_webapp = true;
    }
  } else {
    if (native_window_->type() != NativeWindow::Type::NORMAL) {
      type = NativeWindow::Type::NORMAL;
      reset_webapp = true;
    }
  }

  if (reset_webapp) {
    ResetWebApplication(type);
    // TODO(t.iwanek): there is still problem with rendering content if
    // window is replaced ewk won't render page.
    // This looks the same as for workaround removed in: PR #80
    native_window_->Show();
  }

  if (application_->launched()) {
    application_->AppControl(std::move(appcontrol));
  } else {
    application_->Launch(std::move(appcontrol));
  }
}

void UiRuntime::OnLanguageChanged(const std::string& language) {
  if (application_) {
    application_->OnLanguageChanged();
    elm_language_set(language.c_str());
  }
}

void UiRuntime::OnLowMemory() {
  if (application_) {
    application_->OnLowMemory();
  }
}

int UiRuntime::Exec(int argc, char* argv[]) {
  ui_app_lifecycle_callback_s ops = {NULL, NULL, NULL, NULL, NULL};

  // onCreate
  ops.create = [](void* data) -> bool {
    UiRuntime* ui_runtime = reinterpret_cast<UiRuntime*>(data);
    if (!ui_runtime) {
      LOGGER(ERROR) << "UiRuntime has not been created.";
      return false;
    }
    return ui_runtime->OnCreate();
  };

  // onTerminate
  ops.terminate = [](void* data) -> void {
    UiRuntime* ui_runtime = reinterpret_cast<UiRuntime*>(data);
    if (!ui_runtime) {
      LOGGER(ERROR) << "UiRuntime has not been created.";
      return;
    }
    ui_runtime->OnTerminate();
  };

  // onPause
  ops.pause = [](void* data) -> void {
    UiRuntime* ui_runtime = reinterpret_cast<UiRuntime*>(data);
    if (!ui_runtime) {
      LOGGER(ERROR) << "UiRuntime has not been created.";
      return;
    }
    ui_runtime->OnPause();
  };

  // onResume
  ops.resume = [](void* data) -> void {
    UiRuntime* ui_runtime = reinterpret_cast<UiRuntime*>(data);
    if (!ui_runtime) {
      LOGGER(ERROR) << "UiRuntime has not been created.";
      return;
    }
    ui_runtime->OnResume();
  };

  // onAppControl
  ops.app_control = [](app_control_h app_control, void* data) -> void {
    UiRuntime* ui_runtime = reinterpret_cast<UiRuntime*>(data);
    if (!ui_runtime) {
      LOGGER(ERROR) << "UiRuntime has not been created.";
      return;
    }
    ui_runtime->OnAppControl(app_control);
  };

  // language changed callback
  auto language_changed = [](app_event_info_h event_info, void* user_data) {
    char* str;
    if (app_event_get_language(event_info, &str) == 0 && str != NULL) {
      std::string language = std::string(str);
      std::free(str);
      UiRuntime* ui_runtime = reinterpret_cast<UiRuntime*>(user_data);
      ui_runtime->OnLanguageChanged(language);
    }
  };
  auto low_memory = [](app_event_info_h /*event_info*/, void* user_data) {
    UiRuntime* ui_runtime = reinterpret_cast<UiRuntime*>(user_data);
    ui_runtime->OnLowMemory();
  };
  app_event_handler_h ev_handle;
  ui_app_add_event_handler(&ev_handle,
                           APP_EVENT_LANGUAGE_CHANGED,
                           language_changed,
                           this);
  ui_app_add_event_handler(&ev_handle,
                           APP_EVENT_LOW_MEMORY,
                           low_memory,
                           this);
  STEP_PROFILE_START("ui_app_main -> OnCreate");

  return ui_app_main(argc, argv, &ops, this);
}

}  // namespace runtime
