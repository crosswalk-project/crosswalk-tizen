// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/native_window.h"

#if defined(HAVE_X11)
#include <Ecore_X.h>
#elif defined(HAVE_WAYLAND)
#include <Ecore_Wayland.h>
#endif

#include <cstdint>

#include "common/logger.h"

namespace wrt {

namespace {
  const char* kWRTEdjePath = "/usr/share/edje/wrt/wrt.edj";
  const char* kWinowRotationEventKey = "wm,rotation,changed";
  const char* kWinowFocusedEventKey = "focused";
  const char* kWinowUnfocusedEventKey = "unfocused";
}  // namespace


NativeWindow::NativeWindow()
    : initialized_(false),
      window_(NULL),
      focus_(NULL),
      content_(NULL),
      rotation_(0),
      handler_id_(0) {
}

NativeWindow::~NativeWindow() {
}

void NativeWindow::Initialize() {
  uint16_t pid = getpid();

  // window
  window_ = CreateWindowInternal();
  elm_win_conformant_set(window_, EINA_TRUE);
  int w, h;
#if defined(HAVE_X11)
  ecore_x_window_prop_property_set(
    elm_win_xwindow_get(window_),
    ECORE_X_ATOM_NET_WM_PID,
    ECORE_X_ATOM_CARDINAL, 32, &pid, 1);
  ecore_x_vsync_animator_tick_source_set(elm_win_xwindow_get(window_));
  ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
#elif defined(HAVE_WAYLAND)
  ecore_wl_screen_size_get(&w, &h);
#endif
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
  elm_layout_file_set(top_layout, kWRTEdjePath, "web-application");
  EVAS_SIZE_EXPAND_FILL(top_layout);
  elm_object_content_set(conformant, top_layout);
  evas_object_show(top_layout);

  // focus
  Evas_Object* focus = elm_button_add(top_layout);
  elm_theme_extension_add(NULL, kWRTEdjePath);
  elm_object_style_set(focus, "wrt");
  elm_object_part_content_set(top_layout, "elm.swallow.content", focus);
  EVAS_SIZE_EXPAND_FILL(focus);
  elm_access_object_unregister(focus);
  evas_object_show(focus);
  focus_ = focus;

  // focus callback
  auto focus_callback = [](void* user_data,
                           Evas_Object*,
                           void*) -> void {
    NativeWindow* window = static_cast<NativeWindow*>(user_data);
    window->DidFocusChanged(true);
  };
  auto unfocus_callback = [](void* user_data,
                             Evas_Object*,
                             void*) -> void {
    NativeWindow* window = static_cast<NativeWindow*>(user_data);
    window->DidFocusChanged(false);
  };

  evas_object_smart_callback_add(focus,
                                 kWinowFocusedEventKey,
                                 focus_callback,
                                 this);
  evas_object_smart_callback_add(focus,
                                 kWinowUnfocusedEventKey,
                                 unfocus_callback,
                                 this);

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

  initialized_ = true;
}

void NativeWindow::DidDeleteRequested(void* /*data*/,
    Evas_Object* /*obj*/, void* /*event_info*/) {
  LoggerD("didDeleteRequested");
  elm_exit();
}

void NativeWindow::DidProfileChanged(void* data,
    Evas_Object* /*obj*/, void* /*event_info*/) {
  LoggerD("didProfileChanged");
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
  elm_object_part_content_unset(focus_, "elm.swallow.content");
  elm_object_part_content_set(focus_, "elm.swallow.content", content);
  elm_object_focus_set(focus_, EINA_TRUE);
  content_ = content;
}

void NativeWindow::DidRotation(int degree) {
  rotation_ = degree;
  auto it = handler_table_.begin();
  for ( ; it != handler_table_.end(); ++it) {
    it->second(degree);
  }
}

void NativeWindow::DidFocusChanged(bool got) {
  if (content_ != NULL) {
    elm_object_focus_set(content_, got ? EINA_TRUE : EINA_FALSE);
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
  rotation_ = degree%360;
  elm_win_wm_rotation_preferred_rotation_set(window_, rotation_);
}

void NativeWindow::SetAutoRotation() {
  if (elm_win_wm_rotation_supported_get(window_)) {
    const int rotation[4] = {0, 90, 180, 270};
    elm_win_wm_rotation_available_rotations_set(window_, rotation, 4);
  }
  rotation_ = elm_win_rotation_get(window_);
}

void NativeWindow::Show() {
  evas_object_show(window_);
}

}  // namespace wrt
