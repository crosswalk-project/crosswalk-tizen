// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime.h"

#include "logger.h"
#include "command_line.h"
#include "native_app_window.h"

namespace wrt {

Runtime::Runtime() {

}

Runtime::~Runtime() {
  if (application_) {
    delete application_;
  }
}

bool Runtime::onCreate(void* data) {
  Runtime* runtime = reinterpret_cast<Runtime*>(data);
  if (!runtime) {
    LoggerE("Runtime has not been created.");
    return false;
  }
  std::string appid = CommandLine::ForCurrentProcess()->appid();
  runtime->application_ = new WebApplication(appid);
  if (!runtime->application_) {
    LoggerE("WebApplication couldn't be created.");
    return false;
  }
  return true;
}

void Runtime::onTerminate(void* data) {
}

void Runtime::onPause(void* data) {
  Runtime* runtime = reinterpret_cast<Runtime*>(data);
  if (runtime->application_->initialized()) {
    runtime->application_->Suspend();
  }
}

void Runtime::onResume(void* data) {
  Runtime* runtime = reinterpret_cast<Runtime*>(data);
  if (runtime->application_->initialized()) {
    runtime->application_->Resume();
  }
}

void Runtime::onAppControl(app_control_h app_control, void* data) {
  Runtime* runtime = reinterpret_cast<Runtime*>(data);
  if (runtime->application_->initialized()) {
    // Process AppControl
  } else {
    // Process First Launch
    runtime->createNativeWindow();
    runtime->application_->Launch();
  }
}

int Runtime::Exec(int argc, char* argv[]) {
  ui_app_lifecycle_callback_s ops = {0,};

  ops.create = onCreate;
  ops.terminate = onTerminate;
  ops.pause = onPause;
  ops.resume = onResume;
  ops.app_control = onAppControl;

  return ui_app_main(argc, argv, &ops, this);
}

void Runtime::createNativeWindow() {
  // TODO(wy80.choi) : consider other type of native window.
  native_window_ = new NativeAppWindow();

  native_window_->Initialize();
}

} // namespace wrt

int main(int argc, char* argv[]) {

  // Initalize CommandLineParser
  wrt::CommandLine::Init(argc, argv);

  wrt::Runtime runtime;
  int ret = runtime.Exec(argc, argv);

  return ret;
}
