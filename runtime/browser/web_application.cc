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

#include "runtime/browser/web_application.h"

#include <app.h>
#include <Ecore.h>
#include <ewk_chromium.h>
#include <aul.h>

#include <algorithm>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include "common/application_data.h"
#include "common/app_db.h"
#include "common/app_control.h"
#include "common/command_line.h"
#include "common/locale_manager.h"
#include "common/logger.h"
#include "common/profiler.h"
#include "common/resource_manager.h"
#include "common/string_utils.h"
#include "runtime/browser/native_window.h"
#include "runtime/browser/notification_manager.h"
#include "runtime/browser/popup.h"
#include "runtime/browser/popup_string.h"
#include "runtime/browser/vibration_manager.h"
#include "runtime/browser/web_view.h"
#include "runtime/browser/splash_screen.h"
#include "extensions/common/xwalk_extension_server.h"

#ifndef INJECTED_BUNDLE_PATH
#error INJECTED_BUNDLE_PATH is not set.
#endif

namespace runtime {

namespace {
const char* kKeyNameBack = "back";
const char* kKeyNameMenu = "menu";

const char* kConsoleLogEnableKey = "WRT_CONSOLE_LOG_ENABLE";
const char* kConsoleMessageLogTag = "ConsoleMessage";

const char* kVerboseKey = "verbose";
const char* kPortKey = "port";

const char* kAppControlEventScript =
    "(function(){"
    "var __event = document.createEvent(\"CustomEvent\");\n"
    "__event.initCustomEvent(\"appcontrol\", true, true, null);\n"
    "document.dispatchEvent(__event);\n"
    "\n"
    "for (var i=0; i < window.frames.length; i++)\n"
    "{ window.frames[i].document.dispatchEvent(__event); }"
    "})()";
const char* kBackKeyEventScript =
    "(function(){"
    "var __event = document.createEvent(\"CustomEvent\");\n"
    "__event.initCustomEvent(\"tizenhwkey\", true, true, null);\n"
    "__event.keyName = \"back\";\n"
    "document.dispatchEvent(__event);\n"
    "\n"
    "for (var i=0; i < window.frames.length; i++)\n"
    "{ window.frames[i].document.dispatchEvent(__event); }"
    "})()";
const char* kMenuKeyEventScript =
    "(function(){"
    "var __event = document.createEvent(\"CustomEvent\");\n"
    "__event.initCustomEvent(\"tizenhwkey\", true, true, null);\n"
    "__event.keyName = \"menu\";\n"
    "document.dispatchEvent(__event);\n"
    "\n"
    "for (var i=0; i < window.frames.length; i++)\n"
    "{ window.frames[i].document.dispatchEvent(__event); }"
    "})()";
const char* kAmbientTickEventScript =
    "(function(){"
    "var __event = document.createEvent(\"CustomEvent\");\n"
    "__event.initCustomEvent(\"timetick\", true, true);\n"
    "document.dispatchEvent(__event);\n"
    "\n"
    "for (var i=0; i < window.frames.length; i++)\n"
    "{ window.frames[i].document.dispatchEvent(__event); }"
    "})()";
const char* kFullscreenPrivilege = "http://tizen.org/privilege/fullscreen";
const char* kFullscreenFeature = "fullscreen";
const char* kNotificationPrivilege = "http://tizen.org/privilege/notification";
const char* kLocationPrivilege = "http://tizen.org/privilege/location";
const char* kStoragePrivilege = "http://tizen.org/privilege/unlimitedstorage";
const char* kUsermediaPrivilege = "http://tizen.org/privilege/mediacapture";
const char* kNotiIconFile = "noti_icon.png";
const char* kFileScheme = "file://";

const char* kVisibilitySuspendFeature = "visibility,suspend";
const char* kMediastreamRecordFeature = "mediastream,record";
const char* kEncryptedDatabaseFeature = "encrypted,database";
const char* kRotationLockFeature = "rotation,lock";
const char* kBackgroundMusicFeature = "background,music";
const char* kSoundModeFeature = "sound,mode";
const char* kBackgroundVibrationFeature = "background,vibration";
const char* kCSPFeature = "csp";

const char* kGeolocationPermissionPrefix = "__WRT_GEOPERM_";
const char* kNotificationPermissionPrefix = "__WRT_NOTIPERM_";
const char* kQuotaPermissionPrefix = "__WRT_QUOTAPERM_";
const char* kCertificateAllowPrefix = "__WRT_CERTIPERM_";
const char* kUsermediaPermissionPrefix = "__WRT_USERMEDIAPERM_";
const char* kDBPrivateSection = "private";

const char* kDefaultCSPRule =
    "default-src *; script-src 'self'; style-src 'self'; object-src 'none';";
const char* kResWgtPath = "res/wgt/";
const char* kAppControlMain = "http://tizen.org/appcontrol/operation/main";

bool FindPrivilege(common::ApplicationData* app_data,
                   const std::string& privilege) {
  if (app_data->permissions_info().get() == NULL) return false;
  auto it = app_data->permissions_info()->GetAPIPermissions().begin();
  auto end = app_data->permissions_info()->GetAPIPermissions().end();
  for (; it != end; ++it) {
    if (*it == privilege) return true;
  }
  return false;
}

static void SendDownloadRequest(const std::string& url) {
  common::AppControl request;
  request.set_operation(APP_CONTROL_OPERATION_DOWNLOAD);
  request.set_uri(url);
  request.LaunchRequest();
}

static void InitializeNotificationCallback(Ewk_Context* ewk_context,
                                           WebApplication* app) {
  auto show = [](Ewk_Context*, Ewk_Notification* noti, void* user_data) {
    WebApplication* self = static_cast<WebApplication*>(user_data);
    if (self == NULL) return;
    uint64_t id = ewk_notification_id_get(noti);
    std::string title(ewk_notification_title_get(noti)
                          ? ewk_notification_title_get(noti)
                          : "");
    std::string body(
        ewk_notification_body_get(noti) ? ewk_notification_body_get(noti) : "");
    std::string icon_path = self->data_path() + "/" + kNotiIconFile;
    if (!ewk_notification_icon_save_as_png(noti, icon_path.c_str())) {
      icon_path = "";
    }
    if (NotificationManager::GetInstance()->Show(id, title, body, icon_path))
      ewk_notification_showed(id);
  };
  auto hide = [](Ewk_Context*, uint64_t noti_id, void*) {
    NotificationManager::GetInstance()->Hide(noti_id);
    ewk_notification_closed(noti_id, EINA_FALSE);
  };
  ewk_context_notification_callbacks_set(ewk_context, show, hide, app);
}

static Eina_Bool ExitAppIdlerCallback(void* data) {
  WebApplication* app = static_cast<WebApplication*>(data);
  if (app)
    app->Terminate();
  return ECORE_CALLBACK_CANCEL;
}

static bool ClearCookie(Ewk_Context* ewk_context) {
  Ewk_Cookie_Manager* cookie_manager =
      ewk_context_cookie_manager_get(ewk_context);
  if (!cookie_manager) {
    LOGGER(ERROR) << "Fail to get cookie manager";
    return false;
  }
  ewk_cookie_manager_cookies_clear(cookie_manager);
  return true;
}

static bool ProcessWellKnownScheme(const std::string& url) {
  if (common::utils::StartsWith(url, "file:") ||
      common::utils::StartsWith(url, "app:") ||
      common::utils::StartsWith(url, "data:") ||
      common::utils::StartsWith(url, "http:") ||
      common::utils::StartsWith(url, "https:") ||
      common::utils::StartsWith(url, "widget:") ||
      common::utils::StartsWith(url, "about:") ||
      common::utils::StartsWith(url, "blob:")) {
    return false;
  }

  std::unique_ptr<common::AppControl> request(
      common::AppControl::MakeAppcontrolFromURL(url));
  if (request.get() == NULL || !request->LaunchRequest()) {
    LOGGER(ERROR) << "Fail to send appcontrol request";
    SLoggerE("Fail to send appcontrol request [%s]", url.c_str());
  }

  // Should return true, to stop the WebEngine progress step about this URL
  return true;
}

}  // namespace

WebApplication::WebApplication(
    NativeWindow* window,
    common::ApplicationData* app_data)
    : launched_(false),
      debug_mode_(false),
      verbose_mode_(false),
      ewk_context_(
          ewk_context_new_with_injected_bundle_path(INJECTED_BUNDLE_PATH)),
      has_ownership_of_ewk_context_(true),
      window_(window),
      appid_(app_data->app_id()),
      app_data_(app_data),
      locale_manager_(new common::LocaleManager()),
      terminator_(NULL) {
  Initialize();
}

WebApplication::WebApplication(
    NativeWindow* window,
    common::ApplicationData* app_data,
    Ewk_Context* context)
    : launched_(false),
      debug_mode_(false),
      verbose_mode_(false),
      ewk_context_(context),
      has_ownership_of_ewk_context_(false),
      window_(window),
      appid_(app_data->app_id()),
      app_data_(app_data),
      locale_manager_(new common::LocaleManager()),
      terminator_(NULL) {
  Initialize();
}

WebApplication::~WebApplication() {
  window_->SetContent(NULL);
  auto it = view_stack_.begin();
  for (; it != view_stack_.end(); ++it) {
    delete *it;
  }
  view_stack_.clear();
  if (ewk_context_ && has_ownership_of_ewk_context_)
    ewk_context_delete(ewk_context_);
}

bool WebApplication::Initialize() {
  SCOPE_PROFILE();
  std::unique_ptr<char, decltype(std::free)*> path{app_get_data_path(),
                                                   std::free};
  app_data_path_ = path.get();

  if (app_data_->setting_info() != NULL &&
      app_data_->setting_info()->screen_orientation() ==
          wgt::parse::SettingInfo::ScreenOrientation::AUTO) {
    ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                                kRotationLockFeature, true);
    window_->SetAutoRotation();
  } else if (app_data_->setting_info() != NULL &&
             app_data_->setting_info()->screen_orientation() ==
                 wgt::parse::SettingInfo::ScreenOrientation::PORTRAIT) {
    window_->SetRotationLock(NativeWindow::ScreenOrientation::PORTRAIT_PRIMARY);
  } else if (app_data_->setting_info() != NULL &&
             app_data_->setting_info()->screen_orientation() ==
                 wgt::parse::SettingInfo::ScreenOrientation::LANDSCAPE) {
    window_->SetRotationLock(
        NativeWindow::ScreenOrientation::LANDSCAPE_PRIMARY);
  }

  splash_screen_.reset(new SplashScreen(
      window_, app_data_->splash_screen_info(), app_data_->application_path()));
  resource_manager_.reset(
      new common::ResourceManager(app_data_, locale_manager_.get()));
  resource_manager_->set_base_resource_path(app_data_->application_path());

  auto extension_server = extensions::XWalkExtensionServer::GetInstance();
  extension_server->SetupIPC(ewk_context_);

  // ewk setting
  ewk_context_cache_model_set(ewk_context_, EWK_CACHE_MODEL_DOCUMENT_BROWSER);

  // cookie
  auto cookie_manager = ewk_context_cookie_manager_get(ewk_context_);
  ewk_cookie_manager_accept_policy_set(cookie_manager,
                                       EWK_COOKIE_ACCEPT_POLICY_ALWAYS);

  // set persistent storage path
  std::string cookie_path = data_path() + ".cookie";
  ewk_cookie_manager_persistent_storage_set(
      cookie_manager, cookie_path.c_str(),
      EWK_COOKIE_PERSISTENT_STORAGE_SQLITE);

  // vibration callback
  auto vibration_start_callback = [](uint64_t ms, void*) {
    platform::VibrationManager::GetInstance()->Start(static_cast<int>(ms));
  };
  auto vibration_stop_callback = [](void* /*user_data*/) {
    platform::VibrationManager::GetInstance()->Stop();
  };
  ewk_context_vibration_client_callbacks_set(
      ewk_context_, vibration_start_callback, vibration_stop_callback, NULL);

  auto download_callback = [](const char* downloadUrl, void* /*data*/) {
    SendDownloadRequest(downloadUrl);
  };
  ewk_context_did_start_download_callback_set(ewk_context_, download_callback,
                                              this);
  InitializeNotificationCallback(ewk_context_, this);

  if (FindPrivilege(app_data_, kFullscreenPrivilege)) {
    ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                                kFullscreenFeature, true);
  }

  if (app_data_->setting_info() != NULL &&
      app_data_->setting_info()->background_support_enabled()) {
    ewk_context_tizen_extensible_api_string_set(
                                ewk_context_, kVisibilitySuspendFeature, true);
    ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                                kBackgroundMusicFeature, true);
  } else {
    ewk_context_tizen_extensible_api_string_set(
                               ewk_context_, kVisibilitySuspendFeature, false);
    ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                               kBackgroundMusicFeature, false);
  }
  ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                              kMediastreamRecordFeature, true);
  ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                              kEncryptedDatabaseFeature, true);

  if (app_data_->setting_info() != NULL &&
      app_data_->setting_info()->sound_mode() ==
          wgt::parse::SettingInfo::SoundMode::EXCLUSIVE) {
    ewk_context_tizen_extensible_api_string_set(ewk_context_, kSoundModeFeature,
                                                true);
  }

  if (app_data_->setting_info() != NULL &&
      app_data_->setting_info()->background_vibration()) {
    ewk_context_tizen_extensible_api_string_set(
        ewk_context_, kBackgroundVibrationFeature, true);
  }

  if (app_data_->widget_info() != NULL &&
      !app_data_->widget_info()->default_locale().empty()) {
    locale_manager_->SetDefaultLocale(
        app_data_->widget_info()->default_locale());
  }

  if (app_data_->csp_info() != NULL || app_data_->csp_report_info() != NULL ||
      app_data_->allowed_navigation_info() != NULL) {
    security_model_version_ = 2;
    if (app_data_->csp_info() == NULL ||
        app_data_->csp_info()->security_rules().empty()) {
      csp_rule_ = kDefaultCSPRule;
    } else {
      csp_rule_ = app_data_->csp_info()->security_rules();
    }
    if (app_data_->csp_report_info() != NULL &&
        !app_data_->csp_report_info()->security_rules().empty()) {
      csp_report_rule_ = app_data_->csp_report_info()->security_rules();
    }
    ewk_context_tizen_extensible_api_string_set(ewk_context_, kCSPFeature,
                                                EINA_TRUE);
  } else {
    security_model_version_ = 1;
  }

#ifdef MANUAL_ROTATE_FEATURE_SUPPORT
  // Set manual rotation
  window_->EnableManualRotation(true);
#endif  // MANUAL_ROTATE_FEATURE_SUPPORT

  return true;
}

void WebApplication::Launch(std::unique_ptr<common::AppControl> appcontrol) {
  // send widget info to injected bundle
  ewk_context_tizen_app_id_set(ewk_context_, appid_.c_str());

  // Setup View
  WebView* view = new WebView(window_, ewk_context_);
  SetupWebView(view);

  std::unique_ptr<common::ResourceManager::Resource> res =
      resource_manager_->GetStartResource(appcontrol.get());
  view->SetDefaultEncoding(res->encoding());

  STEP_PROFILE_END("OnCreate -> URL Set");
  STEP_PROFILE_START("URL Set -> Rendered");

  window_->SetContent(view->evas_object());

#ifdef PROFILE_MOBILE
  // rotate and resize window forcibily for landscape mode.
  // window rotate event is generated after window show. so
  // when app get width and height from viewport, wrong value can be returned.
  if (app_data_->setting_info()->screen_orientation() ==
             wgt::parse::SettingInfo::ScreenOrientation::LANDSCAPE) {
    LOGGER(DEBUG) << "rotate and resize window for landscape mode";
    elm_win_rotation_with_resize_set(window_->evas_object(), 270);
    evas_norender(evas_object_evas_get(window_->evas_object()));
  }
#endif  // PROFILE_MOBILE

  view->LoadUrl(res->uri(), res->mime());
  view_stack_.push_front(view);

#ifdef PROFILE_WEARABLE
  // ewk_view_bg_color_set is not working at webview initialization.
  if (app_data_->app_type() == common::ApplicationData::WATCH) {
    view->SetBGColor(0, 0, 0, 255);
  }
#endif  // PROFILE_WEARABLE

  if (appcontrol->data(AUL_K_DEBUG) == "1") {
    debug_mode_ = true;
    LaunchInspector(appcontrol.get());
  }
  if (appcontrol->data(kVerboseKey) == "true") {
    verbose_mode_ = true;
  }

  launched_ = true;
}

void WebApplication::AppControl(
    std::unique_ptr<common::AppControl> appcontrol) {
  std::unique_ptr<common::ResourceManager::Resource> res =
      resource_manager_->GetStartResource(appcontrol.get());

  bool do_reset = res->should_reset();

  if (!do_reset) {
    std::string current_page = view_stack_.front()->GetUrl();
    std::string localized_page =
        resource_manager_->GetLocalizedPath(res->uri());
    if (current_page != localized_page) {
      do_reset = true;
    } else {
      SendAppControlEvent();
    }
  }

  // handle http://tizen.org/appcontrol/operation/main operation specially.
  // only menu-screen app can send launch request with main operation.
  // in this case, web app should have to resume web app not reset.
  if (do_reset && (appcontrol->operation() == kAppControlMain)){
    LOGGER(DEBUG) << "resume app for main operation";
    do_reset = false;
    SendAppControlEvent();
  }

  if (do_reset) {
    // Reset to context
    ClearViewStack();
    WebView* view = view_stack_.front();
    SetupWebView(view);
    view->SetDefaultEncoding(res->encoding());
    view->LoadUrl(res->uri(), res->mime());
    window_->SetContent(view->evas_object());
  }

  if (!debug_mode_ && appcontrol->data(AUL_K_DEBUG) == "1") {
    debug_mode_ = true;
    LaunchInspector(appcontrol.get());
  }
  if (!verbose_mode_ && appcontrol->data(kVerboseKey) == "true") {
    verbose_mode_ = true;
  }
  window_->Active();
}

void WebApplication::SendAppControlEvent() {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->EvalJavascript(kAppControlEventScript);
}

void WebApplication::ClearViewStack() {
  window_->SetContent(NULL);
  WebView* front = view_stack_.front();
  auto it = view_stack_.begin();
  for (; it != view_stack_.end(); ++it) {
    if (*it != front) {
      (*it)->Suspend();
      delete *it;
    }
  }
  view_stack_.clear();
  view_stack_.push_front(front);
}

void WebApplication::Resume() {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->SetVisibility(true);

  if (app_data_->setting_info() != NULL &&
      app_data_->setting_info()->background_support_enabled()) {
    return;
  }

  auto it = view_stack_.begin();
  for (; it != view_stack_.end(); ++it) {
    (*it)->Resume();
  }
}

void WebApplication::Suspend() {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->SetVisibility(false);

  if (app_data_->setting_info() != NULL &&
      app_data_->setting_info()->background_support_enabled()) {
    LOGGER(DEBUG) << "gone background (backgroud support enabed)";
    return;
  }

  auto it = view_stack_.begin();
  for (; it != view_stack_.end(); ++it) {
    (*it)->Suspend();
  }
}

void WebApplication::Terminate() {
  if (terminator_) {
    terminator_();
  } else {
    elm_exit();
  }
  auto extension_server = extensions::XWalkExtensionServer::GetInstance();
  extension_server->Shutdown();
}

void WebApplication::OnCreatedNewWebView(WebView* /*view*/, WebView* new_view) {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->SetVisibility(false);

  SetupWebView(new_view);
  view_stack_.push_front(new_view);
  window_->SetContent(new_view->evas_object());
}

void WebApplication::RemoveWebViewFromStack(WebView* view) {
  if (view_stack_.size() == 0) return;

  WebView* current = view_stack_.front();
  if (current == view) {
    view_stack_.pop_front();
  } else {
    auto found = std::find(view_stack_.begin(), view_stack_.end(), view);
    if (found != view_stack_.end()) {
      view_stack_.erase(found);
    }
  }

  if (view_stack_.size() == 0) {
    Terminate();
  } else if (current != view_stack_.front()) {
    view_stack_.front()->SetVisibility(true);
    window_->SetContent(view_stack_.front()->evas_object());
  }

  // Delete after the callback context(for ewk view) was not used
  ecore_idler_add([](void* view) {
                    WebView* obj = static_cast<WebView*>(view);
                    delete obj;
                    return EINA_FALSE;
                  },
                  view);
}

void WebApplication::OnClosedWebView(WebView* view) {
    RemoveWebViewFromStack(view);
}

void WebApplication::OnReceivedWrtMessage(WebView* /*view*/,
                                          Ewk_IPC_Wrt_Message_Data* msg) {
  Eina_Stringshare* msg_type = ewk_ipc_wrt_message_data_type_get(msg);

#define TYPE_BEGIN(x) (!strncmp(msg_type, x, strlen(x)))
#define TYPE_IS(x) (!strcmp(msg_type, x))

  if (TYPE_BEGIN("xwalk://")) {
    auto extension_server = extensions::XWalkExtensionServer::GetInstance();
    extension_server->HandleIPCMessage(msg);
  } else {
    Eina_Stringshare* msg_id = ewk_ipc_wrt_message_data_id_get(msg);
    Eina_Stringshare* msg_ref_id =
        ewk_ipc_wrt_message_data_reference_id_get(msg);
    Eina_Stringshare* msg_value = ewk_ipc_wrt_message_data_value_get(msg);

    if (TYPE_IS("tizen://hide")) {
      // One Way Message
      window_->InActive();
    } else if (TYPE_IS("tizen://exit")) {
      // One Way Message
      ecore_idler_add(ExitAppIdlerCallback, this);
    } else if (TYPE_IS("tizen://changeUA")) {
      // Async Message
      // Change UserAgent of current WebView
      bool ret = false;
      if (view_stack_.size() > 0 && view_stack_.front() != NULL) {
        ret = view_stack_.front()->SetUserAgent(std::string(msg_value));
      }
      // Send response
      Ewk_IPC_Wrt_Message_Data* ans = ewk_ipc_wrt_message_data_new();
      ewk_ipc_wrt_message_data_type_set(ans, msg_type);
      ewk_ipc_wrt_message_data_reference_id_set(ans, msg_id);
      if (ret)
        ewk_ipc_wrt_message_data_value_set(ans, "success");
      else
        ewk_ipc_wrt_message_data_value_set(ans, "failed");
      if (!ewk_ipc_wrt_message_send(ewk_context_, ans)) {
        LOGGER(ERROR) << "Failed to send response";
      }
      ewk_ipc_wrt_message_data_del(ans);
    } else if (TYPE_IS("tizen://deleteAllCookies")) {
      Ewk_IPC_Wrt_Message_Data* ans = ewk_ipc_wrt_message_data_new();
      ewk_ipc_wrt_message_data_type_set(ans, msg_type);
      ewk_ipc_wrt_message_data_reference_id_set(ans, msg_id);
      if (ClearCookie(ewk_context_))
        ewk_ipc_wrt_message_data_value_set(ans, "success");
      else
        ewk_ipc_wrt_message_data_value_set(ans, "failed");
      if (!ewk_ipc_wrt_message_send(ewk_context_, ans)) {
        LOGGER(ERROR) << "Failed to send response";
      }
      ewk_ipc_wrt_message_data_del(ans);
    } else if (TYPE_IS("tizen://hide_splash_screen")) {
      splash_screen_->HideSplashScreen(SplashScreen::HideReason::CUSTOM);
    }

    eina_stringshare_del(msg_ref_id);
    eina_stringshare_del(msg_id);
    eina_stringshare_del(msg_value);
  }

#undef TYPE_IS
#undef TYPE_BEGIN

  eina_stringshare_del(msg_type);
}

void WebApplication::OnOrientationLock(
    WebView* view, bool lock,
    NativeWindow::ScreenOrientation preferred_rotation) {
  if (view_stack_.size() == 0) return;

  // Only top-most view can set the orientation relate operation
  if (view_stack_.front() != view) return;

  auto orientaion_setting =
      app_data_->setting_info() != NULL
          ? app_data_->setting_info()->screen_orientation()
          :
          wgt::parse::SettingInfo::ScreenOrientation::AUTO;
  if (orientaion_setting != wgt::parse::SettingInfo::ScreenOrientation::AUTO) {
    return;
  }

  if (lock) {
    window_->SetRotationLock(preferred_rotation);
  } else {
    window_->SetAutoRotation();
  }
}

void WebApplication::OnHardwareKey(WebView* view, const std::string& keyname) {
  // NOTE: This code is added to enable back-key on remote URL
  if (!common::utils::StartsWith(view->GetUrl(), kFileScheme)) {
    if (kKeyNameBack == keyname) {
      LOGGER(DEBUG) << "Back to previous page for remote URL";
      if (!view->Backward()) {
        RemoveWebViewFromStack(view_stack_.front());
      }
    }
    return;
  }

  bool enabled = app_data_->setting_info() != NULL
                     ? app_data_->setting_info()->hwkey_enabled()
                     : true;
  if (enabled && kKeyNameBack == keyname) {
    view->EvalJavascript(kBackKeyEventScript);
    // NOTE: This code is added for backward compatibility.
    // If the 'backbutton_presence' is true, WebView should be navigated back.
    if (app_data_->setting_info() &&
        app_data_->setting_info()->backbutton_presence()) {
      if (!view->Backward()) {
        RemoveWebViewFromStack(view_stack_.front());
      }
    }
  } else if (enabled && kKeyNameMenu == keyname) {
    view->EvalJavascript(kMenuKeyEventScript);
  }
}

void WebApplication::OnLanguageChanged() {
  locale_manager_->UpdateSystemLocale();
  ewk_context_cache_clear(ewk_context_);
  auto it = view_stack_.begin();
  for (; it != view_stack_.end(); ++it) {
    (*it)->Reload();
  }
}

void WebApplication::OnConsoleMessage(const std::string& msg, int level) {
  static bool enabled = (getenv(kConsoleLogEnableKey) != NULL);
  enabled = true;

  std::string split_msg = msg;
  std::size_t pos = msg.find(kResWgtPath);
  if (pos != std::string::npos) {
    split_msg = msg.substr(pos + strlen(kResWgtPath));
  }

  if (debug_mode_ || verbose_mode_ || enabled) {
    int dlog_level = DLOG_DEBUG;
    switch (level) {
      case EWK_CONSOLE_MESSAGE_LEVEL_WARNING:
        dlog_level = DLOG_WARN;
        break;
      case EWK_CONSOLE_MESSAGE_LEVEL_ERROR:
        dlog_level = DLOG_ERROR;
        break;
      default:
        dlog_level = DLOG_DEBUG;
        break;
    }
    LOGGER_RAW(dlog_level, kConsoleMessageLogTag)
      << "[" << app_data_->pkg_id() << "] " << split_msg;
  }
}

void WebApplication::OnLowMemory() {
  ewk_context_cache_clear(ewk_context_);
  ewk_context_notify_low_memory(ewk_context_);
}

void WebApplication::OnSoftKeyboardChangeEvent(WebView* /*view*/,
                               SoftKeyboardChangeEventValue softkeyboard_value) {
  LOGGER(DEBUG) << "OnSoftKeyboardChangeEvent";
  std::stringstream script;
  script
    << "(function(){"
    << "var __event = document.createEvent(\"CustomEvent\");\n"
    << "var __detail = {};\n"
    << "__event.initCustomEvent(\"softkeyboardchange\",true,true,__detail);\n"
    << "__event.state = \"" << softkeyboard_value.state << "\";\n"
    << "__event.width = " << softkeyboard_value.width << ";\n"
    << "__event.height = " << softkeyboard_value.height << ";\n"
    << "document.dispatchEvent(__event);\n"
    << "\n"
    << "for (var i=0; i < window.frames.length; i++)\n"
    << "{ window.frames[i].document.dispatchEvent(__event); }"
    << "})()";
  std::string kSoftKeyboardScript = script.str();
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->EvalJavascript(kSoftKeyboardScript.c_str());
}

#ifdef ROTARY_EVENT_FEATURE_SUPPORT
void WebApplication::OnRotaryEvent(WebView* /*view*/,
                                   RotaryEventType type) {
  LOGGER(DEBUG) << "OnRotaryEvent";
  std::stringstream script;
  script
    << "(function(){"
    << "var __event = document.createEvent(\"CustomEvent\");\n"
    << "var __detail = {};\n"
    << "__event.initCustomEvent(\"rotarydetent\", true, true, __detail);\n"
    << "__event.detail.direction = \""
    << (type == RotaryEventType::CLOCKWISE ? "CW" : "CCW")
    << "\";\n"
    << "document.dispatchEvent(__event);\n"
    << "\n"
    << "for (var i=0; i < window.frames.length; i++)\n"
    << "{ window.frames[i].document.dispatchEvent(__event); }"
    << "})()";
  std::string kRotaryEventScript = script.str();
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->EvalJavascript(kRotaryEventScript.c_str());
}
#endif  // ROTARY_EVENT_FEATURE_SUPPORT

void WebApplication::OnTimeTick(long time) {
#if 0
  LOGGER(DEBUG) << "TimeTick";
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->EvalJavascript(kAmbientTickEventScript);
#endif
}

void WebApplication::OnAmbientTick(long time) {
  LOGGER(DEBUG) << "AmbientTick";
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->EvalJavascript(kAmbientTickEventScript);
}

void WebApplication::OnAmbientChanged(bool ambient_mode) {
  LOGGER(DEBUG) << "AmbientChanged";
  std::stringstream script;
  script
    << "(function(){"
    << "var __event = document.createEvent(\"CustomEvent\");\n"
    << "var __detail = {};\n"
    << "__event.initCustomEvent(\"ambientmodechanged\",true,true,__detail);\n"
    << "__event.detail.ambientMode = "
    << (ambient_mode ? "true" : "false") << ";\n"
    << "document.dispatchEvent(__event);\n"
    << "\n"
    << "for (var i=0; i < window.frames.length; i++)\n"
    << "{ window.frames[i].document.dispatchEvent(__event); }"
    << "})()";
  std::string kAmbientChangedEventScript = script.str();
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->EvalJavascript(kAmbientChangedEventScript.c_str());
}

bool WebApplication::OnContextMenuDisabled(WebView* /*view*/) {
  return !(app_data_->setting_info() != NULL
               ? app_data_->setting_info()->context_menu_enabled()
               : true);
}

void WebApplication::OnLoadStart(WebView* /*view*/) {
  LOGGER(DEBUG) << "LoadStart";
}

void WebApplication::OnLoadFinished(WebView* /*view*/) {
  LOGGER(DEBUG) << "LoadFinished";
  splash_screen_->HideSplashScreen(SplashScreen::HideReason::LOADFINISHED);
}

void WebApplication::OnRendered(WebView* /*view*/) {
  STEP_PROFILE_END("URL Set -> Rendered");
  STEP_PROFILE_END("Start -> Launch Completed");
  LOGGER(DEBUG) << "Rendered";
  splash_screen_->HideSplashScreen(SplashScreen::HideReason::RENDERED);

  // Show window after frame rendered.
  window_->Show();
  window_->Active();
}

#ifdef MANUAL_ROTATE_FEATURE_SUPPORT
void WebApplication::OnRotatePrepared(WebView* /*view*/) {
  window_->ManualRotationDone();
}
#endif  // MANUAL_ROTATE_FEATURE_SUPPORT

void WebApplication::LaunchInspector(common::AppControl* appcontrol) {
  unsigned int port = ewk_context_inspector_server_start(ewk_context_, 0);
  std::stringstream ss;
  ss << port;
  std::map<std::string, std::vector<std::string>> data;
  data[kPortKey] = {ss.str()};
  appcontrol->Reply(data);
}

void WebApplication::SetupWebView(WebView* view) {
  view->SetEventListener(this);

  // Setup CSP Rule
  if (security_model_version_ == 2) {
    view->SetCSPRule(csp_rule_, false);
    if (!csp_report_rule_.empty()) {
      view->SetCSPRule(csp_report_rule_, true);
    }
  }
}

bool WebApplication::OnDidNavigation(WebView* /*view*/,
                                     const std::string& url) {
  // scheme handling
  // except(file , http, https, app) pass to appcontrol and return false
  if (ProcessWellKnownScheme(url)) {
    return false;
  }

  // send launch request for blocked URL to guarrenty backward-compatibility.
  if (resource_manager_->AllowNavigation(url)) {
    return true;
  } else {
    LOGGER(DEBUG) << "URL is blocked. send launch request for URL : " << url;
    std::unique_ptr<common::AppControl> request(
      common::AppControl::MakeAppcontrolFromURL(url));
    if (request.get() == NULL || !request->LaunchRequest()) {
      LOGGER(ERROR) << "Fail to send appcontrol request";
    }
    return false;
  }
}

void WebApplication::OnNotificationPermissionRequest(
    WebView*, const std::string& url,
    std::function<void(bool)> result_handler) {
  auto db = common::AppDB::GetInstance();
  std::string reminder =
      db->Get(kDBPrivateSection, kNotificationPermissionPrefix + url);
  if (reminder == "allowed") {
    result_handler(true);
    return;
  } else if (reminder == "denied") {
    result_handler(false);
    return;
  }

  // Local Domain: Grant permission if defined, otherwise Popup user prompt.
  // Remote Domain: Popup user prompt.
  if (common::utils::StartsWith(url, "file://") &&
      FindPrivilege(app_data_, kNotificationPrivilege)) {
    result_handler(true);
    return;
  }

  Popup* popup = Popup::CreatePopup(window_);
  popup->SetButtonType(Popup::ButtonType::AllowDenyButton);
  popup->SetTitle(popup_string::kPopupTitleWebNotification);
  popup->SetBody(popup_string::kPopupBodyWebNotification);
  popup->SetCheckBox(popup_string::kPopupCheckRememberPreference);
  popup->SetResultHandler(
      [db, result_handler, url](Popup* popup, void* /*user_data*/) {
        bool result = popup->GetButtonResult();
        bool remember = popup->GetCheckBoxResult();
        if (remember) {
          db->Set(kDBPrivateSection, kNotificationPermissionPrefix + url,
                  result ? "allowed" : "denied");
        }
        result_handler(result);
      },
      this);
  popup->Show();
}

void WebApplication::OnGeolocationPermissionRequest(
    WebView*, const std::string& url,
    std::function<void(bool)> result_handler) {
  auto db = common::AppDB::GetInstance();
  std::string reminder =
      db->Get(kDBPrivateSection, kGeolocationPermissionPrefix + url);
  if (reminder == "allowed") {
    result_handler(true);
    return;
  } else if (reminder == "denied") {
    result_handler(false);
    return;
  }

  // Local Domain: Grant permission if defined, otherwise block execution.
  // Remote Domain: Popup user prompt if defined, otherwise block execution.
  if (!FindPrivilege(app_data_, kLocationPrivilege)) {
    result_handler(false);
    return;
  }

  if (common::utils::StartsWith(url, "file://")) {
    result_handler(true);
    return;
  }

  Popup* popup = Popup::CreatePopup(window_);
  popup->SetButtonType(Popup::ButtonType::AllowDenyButton);
  popup->SetTitle(popup_string::kPopupTitleGeoLocation);
  popup->SetBody(popup_string::kPopupBodyGeoLocation);
  popup->SetCheckBox(popup_string::kPopupCheckRememberPreference);
  popup->SetResultHandler(
      [db, result_handler, url](Popup* popup, void* /*user_data*/) {
        bool result = popup->GetButtonResult();
        bool remember = popup->GetCheckBoxResult();
        if (remember) {
          db->Set(kDBPrivateSection, kGeolocationPermissionPrefix + url,
                  result ? "allowed" : "denied");
        }
        result_handler(result);
      },
      this);
  popup->Show();
}

void WebApplication::OnQuotaExceed(WebView*, const std::string& url,
                                   std::function<void(bool)> result_handler) {
  auto db = common::AppDB::GetInstance();
  std::string reminder =
      db->Get(kDBPrivateSection, kQuotaPermissionPrefix + url);
  if (reminder == "allowed") {
    result_handler(true);
    return;
  } else if (reminder == "denied") {
    result_handler(false);
    return;
  }

  // Local Domain: Grant permission if defined, otherwise Popup user prompt.
  // Remote Domain: Popup user prompt.
  if (common::utils::StartsWith(url, "file://") &&
      FindPrivilege(app_data_, kStoragePrivilege)) {
    result_handler(true);
    return;
  }

  Popup* popup = Popup::CreatePopup(window_);
  popup->SetButtonType(Popup::ButtonType::AllowDenyButton);
  popup->SetTitle(popup_string::kPopupTitleWebStorage);
  popup->SetBody(popup_string::kPopupBodyWebStorage);
  popup->SetCheckBox(popup_string::kPopupCheckRememberPreference);
  popup->SetResultHandler(
      [db, result_handler, url](Popup* popup, void* /*user_data*/) {
        bool result = popup->GetButtonResult();
        bool remember = popup->GetCheckBoxResult();
        if (remember) {
          db->Set(kDBPrivateSection, kQuotaPermissionPrefix + url,
                  result ? "allowed" : "denied");
        }
        result_handler(result);
      },
      this);
  popup->Show();
}

void WebApplication::OnAuthenticationRequest(
    WebView*, const std::string& /*url*/, const std::string& /*message*/,
    std::function<void(bool submit, const std::string& id,
                       const std::string& password)> result_handler) {
  Popup* popup = Popup::CreatePopup(window_);
  popup->SetButtonType(Popup::ButtonType::LoginCancelButton);
  popup->SetFirstEntry(popup_string::kPopupLabelAuthusername,
                       Popup::EntryType::Edit);
  popup->SetSecondEntry(popup_string::kPopupLabelPassword,
                        Popup::EntryType::PwEdit);
  popup->SetTitle(popup_string::kPopupTitleAuthRequest);
  popup->SetBody(popup_string::kPopupBodyAuthRequest);
  popup->SetResultHandler([result_handler](Popup* popup, void* /*user_data*/) {
                            bool result = popup->GetButtonResult();
                            std::string id = popup->GetFirstEntryResult();
                            std::string passwd = popup->GetSecondEntryResult();
                            result_handler(result, id, passwd);
                          },
                          this);
  popup->Show();
}

void WebApplication::OnCertificateAllowRequest(
    WebView*, const std::string& url, const std::string& pem,
    std::function<void(bool allow)> result_handler) {
  auto db = common::AppDB::GetInstance();
  std::string reminder =
      db->Get(kDBPrivateSection, kCertificateAllowPrefix + pem);
  if (reminder == "allowed") {
    result_handler(true);
    return;
  } else if (reminder == "denied") {
    result_handler(false);
    return;
  }

  Popup* popup = Popup::CreatePopup(window_);
  popup->SetButtonType(Popup::ButtonType::AllowDenyButton);
  popup->SetTitle(popup_string::kPopupTitleCert);
  popup->SetBody(popup_string::GetText(
                 popup_string::kPopupBodyCert) + "\n\n" + url);
  popup->SetCheckBox(popup_string::kPopupCheckRememberPreference);
  popup->SetResultHandler(
      [db, result_handler, pem](Popup* popup, void* /*user_data*/) {
        bool result = popup->GetButtonResult();
        bool remember = popup->GetCheckBoxResult();
        if (remember) {
          db->Set(kDBPrivateSection, kCertificateAllowPrefix + pem,
                  result ? "allowed" : "denied");
        }
        result_handler(result);
      },
      this);
  popup->Show();
}

void WebApplication::OnUsermediaPermissionRequest(
    WebView*, const std::string& url,
    std::function<void(bool)> result_handler) {
  auto db = common::AppDB::GetInstance();
  std::string reminder =
      db->Get(kDBPrivateSection, kUsermediaPermissionPrefix + url);
  if (reminder == "allowed") {
    result_handler(true);
    return;
  } else if (reminder == "denied") {
    result_handler(false);
    return;
  }

  // Local Domain: Grant permission if defined, otherwise block execution.
  // Remote Domain: Popup user prompt if defined, otherwise block execution.
  if (!FindPrivilege(app_data_, kUsermediaPrivilege)) {
    result_handler(false);
    return;
  }

  if (common::utils::StartsWith(url, "file://")) {
    result_handler(true);
    return;
  }

  Popup* popup = Popup::CreatePopup(window_);
  popup->SetButtonType(Popup::ButtonType::AllowDenyButton);
  popup->SetTitle(popup_string::kPopupTitleUserMedia);
  popup->SetBody(popup_string::kPopupBodyUserMedia);
  popup->SetCheckBox(popup_string::kPopupCheckRememberPreference);
  popup->SetResultHandler(
      [db, result_handler, url](Popup* popup, void* /*user_data*/) {
        bool result = popup->GetButtonResult();
        bool remember = popup->GetCheckBoxResult();
        if (remember) {
          db->Set(kDBPrivateSection, kUsermediaPermissionPrefix + url,
                  result ? "allowed" : "denied");
        }
        result_handler(result);
      },
      this);
  popup->Show();
}

}  // namespace runtime
