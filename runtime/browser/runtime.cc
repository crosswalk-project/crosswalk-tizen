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

#include <app.h>
#include <memory>
#include <string>

#include "common/application_data.h"
#include "common/command_line.h"
#include "runtime/common/constants.h"
#include "runtime/browser/runtime.h"
#include "runtime/browser/ui_runtime.h"
#ifdef IME_FEATURE_SUPPORT
#include "runtime/browser/ime_runtime.h"
#endif  // IME_FEATURE_SUPPORT
#ifdef WATCH_FACE_FEATURE_SUPPORT
#include "runtime/browser/watch_runtime.h"
#endif  // WATCH_FACE_FEATURE_SUPPORT

namespace runtime {

bool Runtime::is_on_terminate_called = false;

Runtime::~Runtime() {
}

std::unique_ptr<Runtime> Runtime::MakeRuntime(
    common::ApplicationData* app_data) {
  if (app_data->app_type() == common::ApplicationData::UI) {
    return std::unique_ptr<Runtime>(new UiRuntime(app_data));
  }
#ifdef IME_FEATURE_SUPPORT
  else if (app_data->app_type() == common::ApplicationData::IME) {
    return std::unique_ptr<Runtime>(new ImeRuntime(app_data));
  }
#endif  // IME_FEATURE_SUPPORT
#ifdef WATCH_FACE_FEATURE_SUPPORT
  else if (app_data->app_type() == common::ApplicationData::WATCH) {
    return std::unique_ptr<Runtime>(new WatchRuntime(app_data));
  }
#endif  // WATCH_FACE_FEATURE_SUPPORT
  else {
    return std::unique_ptr<Runtime>(new UiRuntime(app_data));
  }
}

void Runtime::ClosePageFromOnTerminate(WebApplication* app) {
  if (app)
    app->ClosePageFromOnTerminate();
}

}  // namespace runtime
