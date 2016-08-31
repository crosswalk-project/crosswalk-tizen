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

#include "runtime/browser/splash_screen.h"

#include <algorithm>
#include <map>
#include <string>

#include <Ecore_Wayland.h>
#include <Evas_Legacy.h>

#include "common/logger.h"
#include "runtime/browser/native_window.h"
#include "wgt_manifest_handlers/launch_screen_handler.h"

using ScreenOrientation = runtime::NativeWindow::ScreenOrientation;

namespace {

enum class BorderOption { REPEAT = 1, STRETCH, ROUND };

wgt::parse::ScreenOrientation ChooseOrientation(
    const std::map<wgt::parse::ScreenOrientation,
    wgt::parse::LaunchScreenData>& splash_map,
    ScreenOrientation screen_orientation) {
  auto orientation_pair = splash_map.end();

  if (screen_orientation ==
      runtime::NativeWindow::ScreenOrientation::PORTRAIT_PRIMARY) {
     orientation_pair =
         splash_map.find(wgt::parse::ScreenOrientation::PORTRAIT);
  } else {
    orientation_pair =
        splash_map.find(wgt::parse::ScreenOrientation::LANDSCAPE);
  }
  if (orientation_pair == splash_map.end())
        orientation_pair = splash_map.find(wgt::parse::ScreenOrientation::AUTO);

  if (orientation_pair != splash_map.end()) return orientation_pair->first;
  return wgt::parse::ScreenOrientation::NONE;
}

bool ParseImageBorder(const std::vector<std::string>& borders,
                      std::vector<int>* border_values,
                      std::vector<BorderOption>* border_options) {
  std::map<std::string, BorderOption> scaling_string;
  scaling_string["repeat"] = BorderOption::REPEAT;
  scaling_string["stretch"] = BorderOption::STRETCH;
  scaling_string["round"] = BorderOption::ROUND;

  for (const auto& border : borders) {
    std::string::size_type px_index = border.find("px");
    if (px_index != std::string::npos) {
      std::string border_value(border.begin(), border.begin() + px_index);
      border_values->push_back(std::atoi(border_value.c_str()));
    }
    for (const auto& border_string_val : scaling_string) {
      std::string::size_type index = border.find(border_string_val.first);
      if (index != std::string::npos) {
        border_options->push_back(border_string_val.second);
      }
    }
  }

  LOGGER(DEBUG) << "Image border values:";
  for (const auto& border_value : *border_values) {
    LOGGER(DEBUG) << border_value;
  }
  LOGGER(DEBUG) << "Image border scaling values:";
  for (const auto& border_option : *border_options) {
    LOGGER(DEBUG) << static_cast<int>(border_option);
  }

  return !border_values->empty() && !border_options->empty();
}

}  // namespace

namespace runtime {

SplashScreen::SplashScreen(
    runtime::NativeWindow* window,
    std::shared_ptr<const wgt::parse::LaunchScreenInfo> ss_info,
    const std::string& app_path)
    : ss_info_(ss_info),
      window_(window),
      image_(nullptr),
      background_(nullptr),
      background_image_(nullptr),
      is_active_(false) {
  LOGGER(DEBUG) << "start of create splash screen";
  if (ss_info == nullptr) return;
  auto splash_map = ss_info->launch_screen_data();
  auto used_orientation =
      ChooseOrientation(splash_map, window->natural_orientation());
  if (used_orientation == wgt::parse::ScreenOrientation::NONE) return;

  auto dimensions = GetDimensions();

  SetBackground(splash_map[used_orientation], window->evas_object(),
                     dimensions, app_path);
  SetImage(splash_map[used_orientation], window->evas_object(),
                     dimensions, app_path);
  is_active_ = true;
}


void SplashScreen::HideSplashScreen(HideReason reason) {
  if (!is_active_) return;
  if (reason == HideReason::RENDERED &&
      ss_info_->ready_when() != wgt::parse::ReadyWhen::FIRSTPAINT) {
    return;
  }
  if (reason == HideReason::LOADFINISHED &&
      ss_info_->ready_when() != wgt::parse::ReadyWhen::COMPLETE) {
    return;
  }
  if (reason == HideReason::CUSTOM &&
      ss_info_->ready_when() != wgt::parse::ReadyWhen::CUSTOM) {
    return;
  }

  evas_object_hide(background_);
  evas_object_hide(image_);
  evas_object_del(background_);
  evas_object_del(image_);
  background_ = nullptr;
  image_ = nullptr;
  is_active_ = false;
}

std::pair<int, int> SplashScreen::GetDimensions() {
  int w, h;
  ecore_wl_screen_size_get(&w, &h);
  evas_object_resize(background_, w, h);
  return std::make_pair(w, h);
}

void SplashScreen::SetBackground(
    const wgt::parse::LaunchScreenData& splash_data, Evas_Object* parent,
    const SplashScreenBound& bound, const std::string& app_path) {
  background_ = elm_bg_add(parent);
  if (!background_) return;
  evas_object_resize(background_, bound.first, bound.second);

  if (splash_data.background_color != nullptr) {
    elm_bg_color_set(background_,
                     splash_data.background_color->red,
                     splash_data.background_color->green,
                     splash_data.background_color->blue);
  }

  std::vector<int> border_values;
  std::vector<BorderOption> border_options;

  if (!splash_data.background_image.empty() &&
      ParseImageBorder(
          splash_data.image_border, &border_values, &border_options)) {
    const std::string& background_image_path =
        splash_data.background_image.front();

    background_image_ = elm_image_add(background_);
    evas_object_image_file_set(background_image_,
                               (app_path + background_image_path).c_str(),
                               NULL);
    elm_image_aspect_fixed_set(background_image_, 0);
    evas_object_image_border_center_fill_set(background_image_,
                                             EVAS_BORDER_FILL_DEFAULT);

    evas_object_resize(background_image_, bound.first, bound.second);

    int border_l, border_r, border_t, border_b;
    switch (border_values.size()) {
      case 1:
        border_l = border_r = border_t = border_b = -border_values[0];
        break;
      case 2:
        border_t = border_b =  border_values[0];
        border_l = border_r =  border_values[1];
        break;
      case 4:
        border_t = border_values[0];
        border_r = border_values[1];
        border_b = border_values[2];
        border_l = border_values[3];
        break;
      default:
        border_l = border_r = border_t = border_b = 0;
    }
    evas_object_image_border_set(background_image_,
                                 border_l,
                                 border_r,
                                 border_t,
                                 border_b);
    // TODO(a.szulakiewi): add scaling of horizontal and vertical borders
  }

  evas_object_show(background_);
  evas_object_show(background_image_);
}

void SplashScreen::SetImage(
    const wgt::parse::LaunchScreenData& splash_data, Evas_Object* parent,
    const SplashScreenBound& bound, const std::string& app_path) {
  if (!background_) return;
  image_ = elm_image_add(background_);
  if (!image_) return;

  const std::string& image_path = splash_data.image.front();
  elm_image_file_set(image_, (app_path + image_path).c_str(), NULL);
  evas_object_resize(image_, bound.first, bound.second);
  evas_object_show(image_);
}
}  // namespace runtime
