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

#ifndef XWALK_RUNTIME_BROWSER_UI_RUNTIME_H_
#define XWALK_RUNTIME_BROWSER_UI_RUNTIME_H_

#include <app.h>
#include <string>

#include "common/application_data.h"
#include "runtime/browser/runtime.h"
#include "runtime/browser/native_window.h"
#include "runtime/browser/web_application.h"

class Ewk_Context;

namespace runtime {

class UiRuntime : public Runtime {
 public:
  UiRuntime(common::ApplicationData* app_data);
  virtual ~UiRuntime();

  virtual int Exec(int argc, char* argv[]);

 protected:
  virtual bool OnCreate();
  virtual void OnTerminate();
  virtual void OnPause();
  virtual void OnResume();
  virtual void OnAppControl(app_control_h app_control);
  virtual void OnLanguageChanged(const std::string& language);
  virtual void OnLowMemory();

 private:
  void ResetWebApplication(NativeWindow::Type windowType);

  std::unique_ptr<NativeWindow> native_window_;
  std::unique_ptr<WebApplication> application_;
  Ewk_Context* context_;
  common::ApplicationData* app_data_;
};

}  // namespace runtime

#endif  // XWALK_RUNTIME_BROWSER_UI_RUNTIME_H_
