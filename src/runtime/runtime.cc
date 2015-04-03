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

bool Runtime::OnCreate(){
  std::string appid = CommandLine::ForCurrentProcess()->appid();
  application_ = new WebApplication(appid);
  if (!application_) {
    LoggerE("WebApplication couldn't be created.");
    return false;
  }

  // Process First Launch
  createNativeWindow();
  application_->Initialize(native_window_);
  return true;
}

bool Runtime::onCreate(void* data) {
  Runtime* runtime = reinterpret_cast<Runtime*>(data);
  if (!runtime) {
    LoggerE("Runtime has not been created.");
    return false;
  }
  return runtime->OnCreate();
}

void Runtime::OnTerminate(){
}
void Runtime::onTerminate(void* data) {
  Runtime* runtime = reinterpret_cast<Runtime*>(data);
  if (!runtime) {
    LoggerE("Runtime has not been created.");
    return;
  }
  runtime->OnTerminate();
}

void Runtime::OnPause(){
  if (application_->initialized()) {
    application_->Suspend();
  }
}

void Runtime::onPause(void* data) {
  Runtime* runtime = reinterpret_cast<Runtime*>(data);
  if (!runtime) {
    LoggerE("Runtime has not been created.");
    return;
  }
  runtime->OnPause();
}

void Runtime::OnResume(){
  if (application_->initialized()) {
    application_->Resume();
  }
}
void Runtime::onResume(void* data) {
  Runtime* runtime = reinterpret_cast<Runtime*>(data);
  if (!runtime) {
    LoggerE("Runtime has not been created.");
    return;
  }
  runtime->OnResume();
}

void Runtime::OnAppControl(app_control_h app_control){
  if (application_->initialized()) {
    // Process AppControl
    application_->AppControl();
  } else {
    application_->Launch();
  }

}
void Runtime::onAppControl(app_control_h app_control, void* data) {
  Runtime* runtime = reinterpret_cast<Runtime*>(data);
  if (!runtime) {
    LoggerE("Runtime has not been created.");
    return;
  }
  runtime->OnAppControl(app_control);
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
