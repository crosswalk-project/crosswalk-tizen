// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/runtime.h"

#include <ewk_chromium.h>
#include <string>
#include <memory>

#include "common/logger.h"
#include "common/command_line.h"
#include "runtime/native_app_window.h"
#include "runtime/app_control.h"

namespace wrt {

namespace {

static NativeWindow* CreateNativeWindow() {
  // TODO(wy80.choi) : consider other type of native window.
  NativeWindow* window = new NativeAppWindow();
  return window;
}

}  // namespace

Runtime::Runtime()
    : application_(NULL),
      native_window_(NULL) {
}

Runtime::~Runtime() {
  if (application_) {
    delete application_;
  }
  if (native_window_) {
    delete native_window_;
  }
}

bool Runtime::OnCreate() {
  std::string appid = CommandLine::ForCurrentProcess()->appid();

  // Process First Launch
  native_window_ = CreateNativeWindow();
  application_ = new WebApplication(native_window_, appid);
  application_->set_terminator([](){ ui_app_exit(); });
  return true;
}

void Runtime::OnTerminate() {
}

void Runtime::OnPause() {
  if (application_->launched()) {
    application_->Suspend();
  }
}

void Runtime::OnResume() {
  if (application_->launched()) {
    application_->Resume();
  }
}

void Runtime::OnAppControl(app_control_h app_control) {
  std::unique_ptr<AppControl> appcontrol(new AppControl(app_control));

  if (application_->launched()) {
    // Process AppControl
    application_->AppControl(std::move(appcontrol));
  } else {
    application_->Launch(std::move(appcontrol));
  }
}

void Runtime::OnLanguageChanged(const std::string& language) {
  if (application_) {
    application_->OnLanguageChanged();
  }
}

void Runtime::OnLowMemory() {
  if (application_) {
    application_->OnLowMemory();
  }
}

int Runtime::Exec(int argc, char* argv[]) {
  ui_app_lifecycle_callback_s ops = {0, };

  // onCreate
  ops.create = [](void* data) -> bool {
    Runtime* runtime = reinterpret_cast<Runtime*>(data);
    if (!runtime) {
      LoggerE("Runtime has not been created.");
      return false;
    }
    return runtime->OnCreate();
  };

  // onTerminate
  ops.terminate = [](void* data) -> void {
    Runtime* runtime = reinterpret_cast<Runtime*>(data);
    if (!runtime) {
      LoggerE("Runtime has not been created.");
      return;
    }
    runtime->OnTerminate();
  };

  // onPause
  ops.pause = [](void* data) -> void {
    Runtime* runtime = reinterpret_cast<Runtime*>(data);
    if (!runtime) {
      LoggerE("Runtime has not been created.");
      return;
    }
    runtime->OnPause();
  };

  // onResume
  ops.resume = [](void* data) -> void {
    Runtime* runtime = reinterpret_cast<Runtime*>(data);
    if (!runtime) {
      LoggerE("Runtime has not been created.");
      return;
    }
    runtime->OnResume();
  };

  // onAppControl
  ops.app_control = [](app_control_h app_control, void* data) -> void {
    Runtime* runtime = reinterpret_cast<Runtime*>(data);
    if (!runtime) {
      LoggerE("Runtime has not been created.");
      return;
    }
    runtime->OnAppControl(app_control);
  };

  // language changed callback
  auto language_changed = [](app_event_info_h event_info, void* user_data) {
    char* str;
    if (app_event_get_language(event_info, &str) == 0 && str != NULL) {
      std::string language = std::string(str);
      std::free(str);
      Runtime* runtime = reinterpret_cast<Runtime*>(user_data);
      runtime->OnLanguageChanged(language);
    }
  };
  auto low_memory = [](app_event_info_h /*event_info*/, void* user_data) {
    Runtime* runtime = reinterpret_cast<Runtime*>(user_data);
    runtime->OnLowMemory();
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

  return ui_app_main(argc, argv, &ops, this);
}

}  // namespace wrt

int main(int argc, char* argv[]) {
  // Initialize CommandLineParser
  wrt::CommandLine::Init(argc, argv);

  ewk_init();
  char* chromium_arg_options[] = {
    argv[0],
    const_cast<char*>("--enable-file-cookies"),
    const_cast<char*>("--allow-file-access-from-files"),
    const_cast<char*>("--allow-universal-access-from-files")
  };
  const int chromium_arg_cnt =
      sizeof(chromium_arg_options) / sizeof(chromium_arg_options[0]);
  ewk_set_arguments(chromium_arg_cnt, chromium_arg_options);

  wrt::Runtime runtime;
  int ret = runtime.Exec(argc, argv);
  ewk_shutdown();
  return ret;
}
