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

#include <Elementary.h>
#include <inputmethod.h>

#include <memory>
#include <string>

#include "common/application_data.h"
#include "common/app_control.h"
#include "common/app_db.h"
#include "common/command_line.h"
#include "common/logger.h"
#include "common/profiler.h"
#include "runtime/common/constants.h"
#include "runtime/browser/native_ime_window.h"
#include "runtime/browser/preload_manager.h"
#include "runtime/browser/runtime.h"
#include "runtime/browser/ime_runtime.h"

namespace runtime {

namespace {

static NativeWindow* CreateNativeWindow() {
  SCOPE_PROFILE();
  NativeWindow* window = NULL;
  auto cached = PreloadManager::GetInstance()->GetCachedNativeWindow();
  if (cached != nullptr) {
    delete cached;
  }
  window = new NativeImeWindow();
  window->Initialize();

  return window;
}

}  // namespace

ImeRuntime::ImeRuntime(common::ApplicationData* app_data)
    : application_(NULL),
      native_window_(NULL),
      app_data_(app_data) {
}

ImeRuntime::~ImeRuntime() {
  if (application_) {
    delete application_;
  }
  if (native_window_) {
    delete native_window_;
  }
}

static Evas_Object *enter_key_btn = NULL;

static void set_return_key_type(Ecore_IMF_Input_Panel_Return_Key_Type type)
{
  switch (type) {
  case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE:
    elm_object_text_set(enter_key_btn, "Done");
    break;
  case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_GO:
    elm_object_text_set(enter_key_btn, "Go");
    break;
  case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_JOIN:
    elm_object_text_set(enter_key_btn, "Join");
    break;
  case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_LOGIN:
    elm_object_text_set(enter_key_btn, "Login");
    break;
  case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_NEXT:
    elm_object_text_set(enter_key_btn, "Next");
    break;
  case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH:
    elm_object_text_set(enter_key_btn, "Search");
    break;
  case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEND:
    elm_object_text_set(enter_key_btn, "Send");
    break;
  case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN:
    elm_object_text_set(enter_key_btn, "Sign in");
    break;
  default:
    elm_object_text_set(enter_key_btn, "Enter");
    break;
  }
}

void ImeRuntime::OnAppControl() {
  std::unique_ptr<common::AppControl>
      appcontrol(new common::AppControl());
  appcontrol->set_operation("http://tizen.org/appcontrol/operation/default");
  common::AppDB* appdb = common::AppDB::GetInstance();
  appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeBundle,
             appcontrol->encoded_bundle());
  if (application_->launched()) {
    application_->AppControl(std::move(appcontrol));
  } else {
    application_->Launch(std::move(appcontrol));
  }
}

void ImeRuntime::OnCreate() {
  STEP_PROFILE_END("ime_app_main -> OnCreate");
  STEP_PROFILE_END("Start -> OnCreate");
  STEP_PROFILE_START("OnCreate -> URL Set");

  common::CommandLine* cmd = common::CommandLine::ForCurrentProcess();
  std::string appid = cmd->GetAppIdFromCommandLine(kRuntimeExecName);

  // Init AppDB for Runtime
  common::AppDB* appdb = common::AppDB::GetInstance();
  appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeName, "xwalk-tizen");
  appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeAppID, appid);
  if (app_data_->setting_info()->background_support_enabled()) {
    appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeBackgroundSupport, "true");
  } else {
    appdb->Set(kAppDBRuntimeSection, kAppDBRuntimeBackgroundSupport, "false");
  }
  appdb->Remove(kAppDBRuntimeSection, kAppDBRuntimeBundle);

  // Init ImeApplication
  native_window_ = CreateNativeWindow();
  STEP_PROFILE_START("ImeApplication Create");
  application_ = new ImeApplication(native_window_, app_data_);
  STEP_PROFILE_END("ImeApplication Create");

  setlocale(LC_ALL, "");
  bindtextdomain(kTextDomainRuntime, kTextLocalePath);

  LOGGER(DEBUG) << "ime_app_create";
  int w, h;

  Evas_Object *ime_window = native_window_->evas_object();
  if (!ime_window) {
    LOGGER(DEBUG) << "Can't get main window: " << get_last_result();
    return;
  }
  elm_win_screen_size_get(ime_window, NULL, NULL, &w, &h);
  LOGGER(DEBUG) << "w : " << w << ", h : " << h;
  if (w <= 0)
    w = 720;
  if (h <= 0)
    h = 1280;
  ime_set_size(w, h*2/5, h, w*3/5);

  OnAppControl();
}

void ImeRuntime::OnTerminate() {
  LOGGER(DEBUG) << "ime_app_terminate";
  ClosePageFromOnTerminate(application_);
}

void ImeRuntime::OnShow(int context_id, ime_context_h context) {
  Ecore_IMF_Input_Panel_Layout layout;
  ime_layout_variation_e layout_variation;
  int cursor_pos;
  Ecore_IMF_Autocapital_Type autocapital_type;
  Ecore_IMF_Input_Panel_Return_Key_Type return_key_type;
  bool return_key_state, prediction_mode, password_mode;

  LOGGER(DEBUG) << "ime_app_show";

  if (ime_context_get_layout(
        context, &layout) == IME_ERROR_NONE)
    LOGGER(DEBUG) << "layout: " << layout;
  if (ime_context_get_layout_variation(
        context, &layout_variation) == IME_ERROR_NONE)
    LOGGER(DEBUG) << "layout variation: " << layout_variation;
  if (ime_context_get_cursor_position(
        context, &cursor_pos) == IME_ERROR_NONE)
    LOGGER(DEBUG) << "cursor position: " << cursor_pos;
  if (ime_context_get_autocapital_type(
        context, &autocapital_type) == IME_ERROR_NONE)
    LOGGER(DEBUG) << "autocapital_type: " << autocapital_type;
  if (ime_context_get_return_key_type(
        context, &return_key_type) == IME_ERROR_NONE)
    LOGGER(DEBUG) << "return_key_type: " << return_key_type;
  if (ime_context_get_return_key_state(
        context, &return_key_state) == IME_ERROR_NONE)
    LOGGER(DEBUG) << "return_key_state: " << return_key_state;
  if (ime_context_get_prediction_mode(
        context, &prediction_mode) == IME_ERROR_NONE)
    LOGGER(DEBUG) << "prediction_mode: " << prediction_mode;
  if (ime_context_get_password_mode(
        context, &password_mode) == IME_ERROR_NONE)
    LOGGER(DEBUG) << "password_mode: " << password_mode;

  set_return_key_type(return_key_type);
}

void ImeRuntime::OnHide(int context_id) {
  LOGGER(DEBUG) << "ime_app_hide";
}

static void ime_app_focus_in_cb(int ic, void *user_data)
{
  LOGGER(DEBUG) << "focus in: " << ic;
}

static void ime_app_focus_out_cb(int ic, void *user_data)
{
  LOGGER(DEBUG) << "focus out: " << ic;
}

static void ime_app_cursor_position_updated_cb(
    int cursor_pos, void *user_data)
{
  LOGGER(DEBUG) << "cursor position: " << cursor_pos;
}

static void ime_app_return_key_type_set_cb(
    Ecore_IMF_Input_Panel_Return_Key_Type type, void *user_data)
{
  LOGGER(DEBUG) << "Return key type: " << type;

  set_return_key_type(type);
}

static void ime_app_return_key_state_set_cb(
    bool disabled, void *user_data)
{
  LOGGER(DEBUG) << "Return key disabled: " << disabled;
}

static void ime_app_layout_set_cb(
    Ecore_IMF_Input_Panel_Layout layout, void *user_data)
{
  LOGGER(DEBUG) << "layout: " << layout;
}

static bool ime_app_process_key_event_cb(
    ime_key_code_e keycode, ime_key_mask_e keymask,
    ime_device_info_h dev_info, void *user_data)
{
  LOGGER(DEBUG) << "keycode = " << keycode << ", keymask = " << keymask;

  if ((keymask & IME_KEY_MASK_CONTROL) || (keymask & IME_KEY_MASK_ALT) ||
      (keymask & IME_KEY_MASK_META) || (keymask & IME_KEY_MASK_WIN) ||
      (keymask & IME_KEY_MASK_HYPER))
    return false;

  return false;
}

static void ime_app_display_language_changed_cb(
    const char *language, void *user_data)
{
  LOGGER(DEBUG) << "language: " << language;
}

int ImeRuntime::Exec(int argc, char* argv[]) {
  ime_callback_s ops = {NULL, NULL, NULL, NULL};

  // onCreate
  ops.create = [](void* data) -> void {
    runtime::ImeRuntime* ime_runtime =
        reinterpret_cast<runtime::ImeRuntime*>(data);
    if (!ime_runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    ime_runtime->OnCreate();
  };

  // onTerminate
  ops.terminate = [](void* data) -> void {
    runtime::ImeRuntime* ime_runtime =
        reinterpret_cast<runtime::ImeRuntime*>(data);
    if (!ime_runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    ime_runtime->OnTerminate();
  };

  // onShow
  ops.show = [](int context_id, ime_context_h context, void *data) -> void {
    runtime::ImeRuntime* ime_runtime =
        reinterpret_cast<runtime::ImeRuntime*>(data);
    if (!ime_runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    ime_runtime->OnShow(context_id, context);
  };

  // onHide
  ops.hide = [](int context_id, void *data) -> void {
    runtime::ImeRuntime* ime_runtime =
        reinterpret_cast<runtime::ImeRuntime*>(data);
    if (!ime_runtime) {
      LOGGER(ERROR) << "Runtime has not been created.";
      return;
    }
    ime_runtime->OnHide(context_id);
  };

  STEP_PROFILE_START("ime_app_main -> OnCreate");

  // Set the necessary callback functions
  ime_event_set_focus_in_cb(ime_app_focus_in_cb, this);
  ime_event_set_focus_out_cb(ime_app_focus_out_cb, this);
  ime_event_set_cursor_position_updated_cb(
    ime_app_cursor_position_updated_cb, this);
  ime_event_set_layout_set_cb(
    ime_app_layout_set_cb, this);
  ime_event_set_return_key_type_set_cb(
    ime_app_return_key_type_set_cb, this);
  ime_event_set_return_key_state_set_cb(
    ime_app_return_key_state_set_cb, this);
  ime_event_set_process_key_event_cb(
    ime_app_process_key_event_cb, this);
  ime_event_set_display_language_changed_cb(
    ime_app_display_language_changed_cb, this);

  return ime_run(&ops, this);
}


// ime_app_main() function is the main entry point of IME application
// but in case of Web App. ime_app_main() is not a entry point.
// To avoid build fail, add empty function.
#ifdef __cplusplus
extern "C" {
#endif

__attribute__ ((visibility("default")))
void ime_app_main(int argc, char **argv) {
  LOGGER(DEBUG) << "ime_app_main";
}
#ifdef __cplusplus
}
#endif


}  // namespace runtime
