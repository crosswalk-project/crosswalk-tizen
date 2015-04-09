// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/runtime.h"

#include <string>

#include "common/logger.h"
#include "runtime/command_line.h"
#include "runtime/native_app_window.h"

namespace wrt {

namespace {

static NativeWindow* CreateNativeWindow() {
  // TODO(wy80.choi) : consider other type of native window.
  NativeWindow* window = new NativeAppWindow();

  window->Initialize();
  return window;
}

}  // namespace

Runtime::Runtime()
    : application_(NULL) {
}

Runtime::~Runtime() {
  if (application_) {
    delete application_;
  }
}

bool Runtime::OnCreate() {
  std::string appid = CommandLine::ForCurrentProcess()->appid();
  application_ = new WebApplication(appid);
  if (!application_) {
    LoggerE("WebApplication couldn't be created.");
    return false;
  }

  // Process First Launch
  native_window_ = CreateNativeWindow();
  application_->Initialize(native_window_);
  return true;
}

void Runtime::OnTerminate() {
}

void Runtime::OnPause() {
  if (application_->initialized()) {
    application_->Suspend();
  }
}

void Runtime::OnResume() {
  if (application_->initialized()) {
    application_->Resume();
  }
}

void Runtime::OnAppControl(app_control_h app_control) {
  if (application_->initialized()) {
    // Process AppControl
    application_->AppControl();
  } else {
    application_->Launch();
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

  return ui_app_main(argc, argv, &ops, this);
}

}  // namespace wrt

int main(int argc, char* argv[]) {
  // Initalize CommandLineParser
  wrt::CommandLine::Init(argc, argv);

  wrt::Runtime runtime;
  int ret = runtime.Exec(argc, argv);

  return ret;
}
