// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "native_app_window.h"

#include <Elementary.h>

namespace wrt {

NativeAppWindow::NativeAppWindow() {
}

NativeAppWindow::~NativeAppWindow() {
}

Evas_Object* NativeAppWindow::createWindowInternal() {
  elm_config_preferred_engine_set("opengl_x11");
  return elm_win_add(NULL, "wrt-widget", ELM_WIN_BASIC);
}


} // namespace wrt
