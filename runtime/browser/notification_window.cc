/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
#include <runtime/browser/notification_window.h>

#include <Ecore_Wayland.h>
#include <efl_util.h>

namespace runtime {


NotificationWindow::NotificationWindow() {
  window_type_ = NativeWindow::Type::NOTIFICATION;
}

Evas_Object* NotificationWindow::CreateWindowInternal() {
  elm_config_accel_preference_set("opengl");
  Evas_Object* window = elm_win_add(NULL, "xwalk-window", ELM_WIN_NOTIFICATION);
  efl_util_set_notification_window_level(window,
                                         EFL_UTIL_NOTIFICATION_LEVEL_3);
  SetAlwaysOnTop(window, true);
  return window;
}

void NotificationWindow::SetAlwaysOnTop(Evas_Object* window, bool enable) {
  if (enable)
    elm_win_aux_hint_add(window, "wm.policy.win.above.lock", "1");
  else
    elm_win_aux_hint_add(window, "wm.policy.win.above.lock", "0");
}


}  // namespace runtime
