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
  const char* kWRTEdjePath = "/usr/share/edje/wrt/Wrt.edj";
}  // namespace

NativeWindow::NativeWindow()
    : initialized_(false),
      window_(NULL) {
}

NativeWindow::~NativeWindow() {
}

void NativeWindow::Initialize() {
  uint16_t pid = getpid();

  // window
  window_ = createWindowInternal();
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
                                 didDeleteRequested, this);
  evas_object_smart_callback_add(window_, "profile,changed",
                                 didProfileChanged, this);

  #define EVAS_SIZE_EXPAND_FILL(obj) \
    evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); \
    evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);

  // background
  Evas_Object* bg = evas_object_rectangle_add(evas_object_evas_get(window_));
  evas_object_color_set(bg, 0, 0, 0, 0);
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
  elm_layout_theme_set(top_layout, "layout", "application", "default");
  EVAS_SIZE_EXPAND_FILL(top_layout);
  elm_object_content_set(conformant, top_layout);
  evas_object_show(top_layout);

  // naviframe
  Evas_Object* naviframe = elm_naviframe_add(top_layout);
  EVAS_SIZE_EXPAND_FILL(naviframe);
  elm_object_part_content_set(top_layout, "elm.swallow.content", naviframe);
  evas_object_show(naviframe);

  // main layout
  Evas_Object* main_layout = elm_layout_add(naviframe);
  elm_layout_file_set(main_layout, kWRTEdjePath, "web-application");
  EVAS_SIZE_EXPAND_FILL(main_layout);
  Elm_Object_Item* navi_item = elm_naviframe_item_push(
      naviframe, NULL, NULL, NULL, main_layout, NULL);
  elm_naviframe_item_title_enabled_set(navi_item, EINA_FALSE, EINA_FALSE);
  // elm_naviframe_item_pop_cb_set(navi_item, naviframeItemPopCallback, NULL);
  evas_object_show(main_layout);

  // focus
  Evas_Object* focus = elm_button_add(main_layout);
  elm_theme_extension_add(NULL, kWRTEdjePath);
  elm_object_style_set(focus, "wrt");
  elm_object_part_content_set(main_layout, "elm.swallow.content", focus);
  EVAS_SIZE_EXPAND_FILL(focus);
  elm_access_object_unregister(focus);
  evas_object_show(focus);

  initialized_ = true;
}

void NativeWindow::didDeleteRequested(void* /*data*/,
    Evas_Object* /*obj*/, void* /*event_info*/) {
  LoggerD("didDeleteRequested");
  elm_exit();
}

void NativeWindow::didProfileChanged(void* data,
    Evas_Object* /*obj*/, void* /*event_info*/) {
  LoggerD("didProfileChanged");
}

Evas_Object* NativeWindow::evas_object() const {
  return window_;
}

void NativeWindow::SetContent(Evas_Object* content) {
  // TODO(sngn.lee): swallow content into focus
}



}  // namespace wrt
