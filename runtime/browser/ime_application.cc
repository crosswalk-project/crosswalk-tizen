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

#include <memory>

#include "common/application_data.h"
#include "common/logger.h"
#include "runtime/browser/native_window.h"
#include "runtime/browser/web_view.h"
#include "runtime/browser/ime_application.h"

namespace runtime {

ImeApplication::ImeApplication(
    NativeWindow* window, common::ApplicationData* app_data)
    : WebApplication(window, app_data) {
}

ImeApplication::~ImeApplication() {
}

// override
void ImeApplication::OnLoadFinished(WebView* view) {
  this->WebApplication::OnLoadFinished(view);

  LOGGER(DEBUG) << "LoadFinished";
  const char* kImeActivateFunctionCallScript =
      "(function(){WebHelperClient.impl.activate();})()";
  view->EvalJavascript(kImeActivateFunctionCallScript);
}

}  // namespace runtime
