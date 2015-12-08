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

#ifndef XWALK_RUNTIME_BROWSER_SPLASH_SCREEN_H_
#define XWALK_RUNTIME_BROWSER_SPLASH_SCREEN_H_

#include <map>
#include <memory>
#include <utility>
#include <string>
#include <vector>

#include <Elementary.h>

#include "manifest_handlers/splash_screen_handler.h"
#include "runtime/browser/native_window.h"

namespace runtime {

class SplashScreen {
 public:
  typedef std::pair<int, int> SplashScreenBound;
  enum class HideReason { RENDERED, LOADFINISHED, CUSTOM };

  SplashScreen(NativeWindow* window,
               std::shared_ptr<const wgt::parse::SplashScreenInfo> ss_info,
               const std::string& app_path);
  void HideSplashScreen(HideReason reason);

 private:
  std::pair<int, int> GetDimensions();
  void SetBackground(const wgt::parse::SplashScreenData& splash_data,
                     Evas_Object* parent, const SplashScreenBound& bound,
                     const std::string& app_path);

  void SetImage(const wgt::parse::SplashScreenData& splash_data,
                Evas_Object* parent, const SplashScreenBound& bound,
                const std::string& app_path);

  std::shared_ptr<const wgt::parse::SplashScreenInfo> ss_info_;
  NativeWindow* window_;
  Evas_Object* image_;
  Evas_Object* background_;
  Evas_Object* background_image_;
  bool is_active_;
};
}  // namespace runtime
#endif  // XWALK_RUNTIME_BROWSER_SPLASH_SCREEN_H_
