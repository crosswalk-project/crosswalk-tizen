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

#include "runtime/browser/native_watch_window.h"
#include "common/logger.h"

#include <watch_app_efl.h>
#include <Elementary.h>

namespace runtime {

NativeWatchWindow::NativeWatchWindow() {
}

NativeWatchWindow::~NativeWatchWindow() {
}

Evas_Object* NativeWatchWindow::CreateWindowInternal() {
  Evas_Object* window = NULL;
  elm_config_accel_preference_set("opengl");
  watch_app_get_elm_win(&window);
  elm_win_alpha_set(window, EINA_TRUE);
  evas_object_render_op_set(window, EVAS_RENDER_COPY);
  return window;
}


}  // namespace runtime
