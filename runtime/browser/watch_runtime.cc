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

#include <ewk_chromium.h>
#include <watch_app.h>
#include <appcore-watch.h>

#include <memory>
#include <string>

#include "common/application_data.h"
#include "common/app_control.h"
#include "common/app_db.h"
#include "common/command_line.h"
#include "common/logger.h"
#include "common/profiler.h"
#include "runtime/common/constants.h"
#include "runtime/browser/native_watch_window.h"
#include "runtime/browser/preload_manager.h"
#include "runtime/browser/runtime.h"
#include "runtime/browser/watch_runtime.h"

namespace runtime {

namespace {

static NativeWindow* CreateNativeWindow() {
  SCOPE_PROFILE();
  NativeWindow* window = NULL;
  auto cached = PreloadManager::GetInstance()->GetCachedNativeWindow();
  if (cached != nullptr) {
    delete cached;
  }
  window = new NativeWatchWindow();
  window->Initialize();

  return window;
}

}  // namespace

WatchRuntime::WatchRuntime(common::ApplicationData* app_data)
    : application_(NULL),
      native_window_(NULL),
      app_data_(app_data) {
}

WatchRuntime::~WatchRuntime() {
  if (application_) {
    delete application_;
  }
  if (native_window_) {
    delete native_window_;
  }
}

bool WatchRuntime::OnCreate() {
  STEP_PROFILE_END("watch_app_main -> OnCreate");
  STEP_PROFILE_END("Start -> OnCreate");
  STEP_PROFILE_START("OnCreate -> URL Set");

  common::CommandLine* cmd = common::CommandLine::ForCurrentProcess();
  std::string appid = cmd->GetAppIdFromCommandLine(kRuntimeExecName);

  // Init AppDB for Runtime
  common::AppDB* appdb = common::AppDB::GetInstance();
  appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeName, "xwalk-tizen");
  appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeAppID, appid);
  appdb->Remove(kAppDBRuntimeSection, kAppDBRuntimeBundle);

  // Init WebApplication
  native_window_ = CreateNativeWindow();
  STEP_PROFILE_START("WebApplication Create");
  application_ = new WebApplication(native_window_, app_data_);
  STEP_PROFILE_END("WebApplication Create");
  application_->set_terminator([](){ watch_app_exit(); });

  setlocale(LC_ALL, "");
  bindtextdomain(kTextDomainRuntime, kTextLocalePath);

  return true;
}

void WatchRuntime::OnTerminate() {
  if (application_) {
    delete application_;
    application_ = nullptr;
  }
  if (native_window_) {
    delete native_window_;
    native_window_ = nullptr;
  }
}

void WatchRuntime::OnPause() {
  if (application_->launched()) {
    application_->Suspend();
  }
}

void WatchRuntime::OnResume() {
  if (application_->launched()) {
    application_->Resume();
  }
}

void WatchRuntime::OnAppControl(app_control_h app_control) {
  SCOPE_PROFILE();
  std::unique_ptr<common::AppControl>
      appcontrol(new common::AppControl(app_control));
  common::AppDB* appdb = common::AppDB::GetInstance();
  appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeBundle,
             appcontrol->encoded_bundle());
  if (application_->launched()) {
    application_->AppControl(std::move(appcontrol));
  } else {
    application_->Launch(std::move(appcontrol));
  }
}

void WatchRuntime::OnLanguageChanged(const std::string& language) {
  if (application_) {
    application_->OnLanguageChanged();
    elm_language_set(language.c_str());
  }
}

void WatchRuntime::OnLowMemory() {
  if (application_) {
    application_->OnLowMemory();
  }
}

void WatchRuntime::OnTimeTick(watch_time_h watch_time) {
  //do not fire tick event for normal clock for web app
#if 0
  time_t time;
  int ret = watch_time_get_utc_timestamp(watch_time, &time);
  if (!ret) {
    LOGGER(DEBUG) << "time : " << time;
    application_->OnTimeTick(time);
  } else {
    LOGGER(DEBUG) << "Fail to get utc time. skip send time tick event";
  }
#endif
}

void WatchRuntime::OnAmbientTick(watch_time_h watch_time) {
  time_t time;
  int ret = watch_time_get_utc_timestamp(watch_time, &time);
  if (!ret) {
    LOGGER(DEBUG) << "time : " << time;
    application_->OnAmbientTick(time);
  } else {
    LOGGER(ERROR) << "Fail to get utc time. skip send ambient tick event";
  }
}

void WatchRuntime::OnAmbientChanged(bool ambient_mode) {
  application_->OnAmbientChanged(ambient_mode);
}

int WatchRuntime::Exec(int argc, char* argv[]) {
  watch_app_lifecycle_callback_s ops =
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

  // onCreate
  ops.create = [](int width, int height, void *data) -> bool {
    WatchRuntime* runtime = reinterpret_cast<WatchRuntime*>(data);
    if (!runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return false;
    }
    return runtime->OnCreate();
  };

  // onTerminate
  ops.terminate = [](void* data) -> void {
    WatchRuntime* runtime = reinterpret_cast<WatchRuntime*>(data);
    if (!runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    runtime->OnTerminate();
  };

  // onPause
  ops.pause = [](void* data) -> void {
    WatchRuntime* runtime = reinterpret_cast<WatchRuntime*>(data);
    if (!runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    runtime->OnPause();
  };

  // onResume
  ops.resume = [](void* data) -> void {
    WatchRuntime* runtime = reinterpret_cast<WatchRuntime*>(data);
    if (!runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    runtime->OnResume();
  };

  // onAppControl
  ops.app_control = [](app_control_h app_control, void* data) -> void {
    WatchRuntime* runtime = reinterpret_cast<WatchRuntime*>(data);
    if (!runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    runtime->OnAppControl(app_control);
  };

  // onTimeTick
  ops.time_tick = [](watch_time_h watch_time, void* data) -> void {
    WatchRuntime* runtime = reinterpret_cast<WatchRuntime*>(data);
    if (!runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    runtime->OnTimeTick(watch_time);
  };

  // onAmbientTick
  ops.ambient_tick = [](watch_time_h watch_time, void* data) -> void {
    WatchRuntime* runtime = reinterpret_cast<WatchRuntime*>(data);
    if (!runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    runtime->OnAmbientTick(watch_time);
  };

  // onAmbientChanged
  ops.ambient_changed = [](bool ambient_mode, void* data) -> void {
    WatchRuntime* runtime = reinterpret_cast<WatchRuntime*>(data);
    if (!runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    // To render web application on ambient mode
    if (ambient_mode) {
      runtime->OnResume();
    } else {
      runtime->OnPause();
    }
    runtime->OnAmbientChanged(ambient_mode);
  };

  // language changed callback
  auto language_changed = [](app_event_info_h event_info, void* user_data) {
    char* str;
    if (app_event_get_language(event_info, &str) == 0 && str != NULL) {
      std::string language = std::string(str);
      std::free(str);
      WatchRuntime* runtime = reinterpret_cast<WatchRuntime*>(user_data);
      runtime->OnLanguageChanged(language);
    }
  };

  auto low_memory = [](app_event_info_h /*event_info*/, void* user_data) {
    WatchRuntime* runtime = reinterpret_cast<WatchRuntime*>(user_data);
    runtime->OnLowMemory();
  };

  app_event_handler_h ev_handle;
  watch_app_add_event_handler(&ev_handle,
                           APP_EVENT_LANGUAGE_CHANGED,
                           language_changed,
                           this);
  watch_app_add_event_handler(&ev_handle,
                           APP_EVENT_LOW_MEMORY,
                           low_memory,
                           this);
  STEP_PROFILE_START("watch_app_main -> OnCreate");

  return watch_app_main(argc, argv, &ops, this);
}

}  // namespace runtime
