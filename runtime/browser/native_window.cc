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

#include "runtime/browser/native_window.h"

#include <Ecore_Wayland.h>
#include <cstdint>

#include "common/logger.h"

namespace runtime {

namespace {
  const char* kEdjePath = "/usr/share/edje/xwalk/xwalk_tizen.edj";
  const char* kWinowRotationEventKey = "wm,rotation,changed";
}  // namespace


NativeWindow::NativeWindow()
    : initialized_(false),
      window_(NULL),
      layout_(NULL),
      content_(NULL),
      rotation_(0),
      handler_id_(0) {
}

NativeWindow::~NativeWindow() {
}

void NativeWindow::Initialize() {
  // window
  window_ = CreateWindowInternal();
  elm_win_conformant_set(window_, EINA_TRUE);
  int w, h;
  ecore_wl_screen_size_get(&w, &h);
  evas_object_resize(window_, w, h);
  elm_win_autodel_set(window_, EINA_TRUE);
  evas_object_smart_callback_add(window_, "delete,request",
                                 DidDeleteRequested, this);
  evas_object_smart_callback_add(window_, "profile,changed",
                                 DidProfileChanged, this);

  #define EVAS_SIZE_EXPAND_FILL(obj) \
    evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); \
    evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);

  // background
  Evas_Object* bg = evas_object_rectangle_add(evas_object_evas_get(window_));
  evas_object_color_set(bg, 0, 0, 0, 255);
  EVAS_SIZE_EXPAND_FILL(bg);
  elm_win_resize_object_add(window_, bg);
  evas_object_render_op_set(bg, EVAS_RENDER_BLEND);
  evas_object_show(bg);

  // conformant
  Evas_Object* conformant = elm_conformant_add(window_);
  EVAS_SIZE_EXPAND_FILL(conformant);
  elm_win_resize_object_add(window_, conformant);
  evas_object_show(conformant);

  // top layout
  Evas_Object* top_layout = elm_layout_add(conformant);
  elm_layout_file_set(top_layout, kEdjePath, "web-application");
  EVAS_SIZE_EXPAND_FILL(top_layout);
  elm_object_content_set(conformant, top_layout);
  evas_object_show(top_layout);
  layout_ = top_layout;

  // Rotation
  auto rotation_callback = [](void* user_data,
                              Evas_Object* obj,
                              void*) -> void {
      NativeWindow* window = static_cast<NativeWindow*>(user_data);
      int degree = elm_win_rotation_get(obj);
      window->DidRotation(degree);
  };
  evas_object_smart_callback_add(window_,
                                 kWinowRotationEventKey,
                                 rotation_callback,
                                 this);

  if (w > h) {
    natural_orientation_ = ScreenOrientation::LANDSCAPE_PRIMARY;
  } else {
    natural_orientation_ = ScreenOrientation::PORTRAIT_PRIMARY;
  }

  initialized_ = true;
}

void NativeWindow::DidDeleteRequested(void* /*data*/,
    Evas_Object* /*obj*/, void* /*event_info*/) {
  LOGGER(DEBUG) << "didDeleteRequested";
  elm_exit();
}

void NativeWindow::DidProfileChanged(void* /*data*/,
    Evas_Object* /*obj*/, void* /*event_info*/) {
  LOGGER(DEBUG) << "didProfileChanged";
}

Evas_Object* NativeWindow::evas_object() const {
  return window_;
}

void NativeWindow::SetContent(Evas_Object* content) {
  // Remarks
  // If any object was already set as a content object in the same part,
  // the previous object will be deleted automatically with this call.
  // If the content is NULL, this call will just delete the previous object.
  // If the If you wish to preserve it,
  // issue elm_object_part_content_unset() on it first.

  evas_object_show(content);
  elm_object_part_content_unset(layout_, "elm.swallow.content");
  elm_object_part_content_set(layout_, "elm.swallow.content", content);
  content_ = content;

  // attached webview was resized by evas_norender API
  evas_norender(evas_object_evas_get(window_));
}

void NativeWindow::DidRotation(int degree) {
  rotation_ = degree;
  auto it = handler_table_.begin();
  for ( ; it != handler_table_.end(); ++it) {
    it->second(degree);
  }
}

int NativeWindow::AddRotationHandler(RotationHandler handler) {
  int id = handler_id_++;
  handler_table_[id] = handler;
  return id;
}

void NativeWindow::RemoveRotationHandler(int id) {
  handler_table_.erase(id);
}

void NativeWindow::SetRotationLock(int degree) {
  if (degree != -1)
    rotation_ = degree % 360;
  elm_win_wm_rotation_preferred_rotation_set(window_, rotation_);
}

void NativeWindow::SetRotationLock(ScreenOrientation orientation) {
  int portrait_natural_angle[] = {
    0,  // PORTRAIT_PRIMARY
    180,  // PORTRAIT_SECONDARY
    270,  // LANDSCAPE_PRIMARY
    90,  // LANDSCAPE_SECONDARY
    0,  // NATURAL
    -1  // ANY
  };
  int landscape_natural_angle[] = {
    270,  // PORTRAIT_PRIMARY
    90,  // PORTRAIT_SECONDARY
    0,  // LANDSCAPE_PRIMARY
    180,  // LANDSCAPE_SECONDARY
    0,  // NATURAL
    -1,  // ANY
  };
  auto& convert_table =
      natural_orientation_ == ScreenOrientation::PORTRAIT_PRIMARY ?
          portrait_natural_angle :
          landscape_natural_angle;
  SetRotationLock(convert_table[static_cast<int>(orientation)]);
}


void NativeWindow::SetAutoRotation() {
  elm_win_wm_rotation_preferred_rotation_set(window_, -1);
  if (elm_win_wm_rotation_supported_get(window_)) {
    const int rotation[4] = {0, 90, 180, 270};
    elm_win_wm_rotation_available_rotations_set(window_, rotation, 4);
  }
  rotation_ = elm_win_rotation_get(window_);
}

void NativeWindow::Show() {
  evas_object_show(window_);
}

void NativeWindow::Active() {
  elm_win_activate(window_);
}

void NativeWindow::InActive() {
  elm_win_lower(window_);
}

void NativeWindow::FullScreen(bool enable) {
  elm_win_indicator_opacity_set(window_,
      enable ? ELM_WIN_INDICATOR_TRANSPARENT : ELM_WIN_INDICATOR_OPAQUE);
}

}  // namespace runtime
