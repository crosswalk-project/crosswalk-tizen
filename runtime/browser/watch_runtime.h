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

#ifndef XWALK_RUNTIME_BROWSER_WATCH_RUNTIME_H_
#define XWALK_RUNTIME_BROWSER_WATCH_RUNTIME_H_

#include <app.h>
#include <watch_app.h>
#include <string>

#include "common/application_data.h"
#include "runtime/browser/runtime.h"
#include "runtime/browser/native_window.h"
#include "runtime/browser/web_application.h"

namespace runtime {

class WatchRuntime : public Runtime {
 public:
  WatchRuntime(common::ApplicationData* app_data);
  virtual ~WatchRuntime();

  virtual int Exec(int argc, char* argv[]);

 protected:
  virtual bool OnCreate();
  virtual void OnTerminate();
  virtual void OnPause();
  virtual void OnResume();
  virtual void OnAppControl(app_control_h app_control);
  virtual void OnLanguageChanged(const std::string& language);
  virtual void OnLowMemory();
  virtual void OnTimeTick(watch_time_h watch_time);
  virtual void OnAmbientTick(watch_time_h watch_time);
  virtual void OnAmbientChanged(bool ambient_mode);

 private:
  WebApplication* application_;
  NativeWindow* native_window_;
  common::ApplicationData* app_data_;
};

}  // namespace runtime

#endif  // XWALK_RUNTIME_BROWSER_WATCH_RUNTIME_H_
