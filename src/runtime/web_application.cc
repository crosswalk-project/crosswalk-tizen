// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/web_application.h"

#include <app.h>
#include <Ecore.h>
#include <ewk_chromium.h>
#include <algorithm>
#include <memory>
#include <sstream>
#include <vector>
#include <map>

#include "common/logger.h"
#include "common/command_line.h"
#include "common/string_utils.h"
#include "runtime/native_window.h"
#include "runtime/web_view.h"
#include "runtime/vibration_manager.h"
#include "runtime/app_control.h"
#include "runtime/locale_manager.h"
#include "runtime/application_data.h"
#include "runtime/resource_manager.h"

namespace {
  // TODO(sngn.lee) : It should be declare in common header
  const char* kKeyNameBack = "back";

  const char* kConsoleLogEnableKey = "WRT_CONSOLE_LOG_ENABLE";
  const char* kConsoleMessageLogTag = "ConsoleMessage";

  const char* kDebugKey = "debug";
  const char* kPortKey = "port";

  // TODO(wy80.choi): consider 64bits system.
  const char* kInjectedBundlePath = "/usr/lib/libwrt-injected-bundle.so";

  const char* kAppControlEventScript = \
        "(function(){"
        "var __event = document.createEvent(\"CustomEvent\");\n"
        "__event.initCustomEvent(\"appcontrol\", true, true);\n"
        "document.dispatchEvent(__event);\n"
        "\n"
        "for (var i=0; i < window.frames.length; i++)\n"
        "{ window.frames[i].document.dispatchEvent(__event); }"
        "})()";
  const char* kBackKeyEventScript = \
        "(function(){"
        "var __event = document.createEvent(\"CustomEvent\");\n"
        "__event.initCustomEvent(\"tizenhwkey\", true, true);\n"
        "__event.keyName = \"back\";\n"
        "document.dispatchEvent(__event);\n"
        "\n"
        "for (var i=0; i < window.frames.length; i++)\n"
        "{ window.frames[i].document.dispatchEvent(__event); }"
        "})()";
}  // namespace

namespace wrt {

WebApplication::WebApplication(
    NativeWindow* window, std::unique_ptr<ApplicationData> app_data)
    : launched_(false),
      debug_mode_(false),
      ewk_context_(ewk_context_new_with_injected_bundle_path(
          kInjectedBundlePath)),
      window_(window),
      appid_(app_data->app_id()),
      locale_manager_(new LocaleManager()),
      app_data_(std::move(app_data)),
      terminator_(NULL),
      uuid_(utils::GenerateUUID()) {
  std::unique_ptr<char, decltype(std::free)*>
    path {app_get_data_path(), std::free};
  app_data_path_ = path.get();

  resource_manager_.reset(
      new ResourceManager(app_data_.get(), locale_manager_.get()));
  resource_manager_->set_base_resource_path(
      app_data_->application_path());
  Initialize();
}

WebApplication::~WebApplication() {
  if (ewk_context_)
    ewk_context_delete(ewk_context_);
}

bool WebApplication::Initialize() {
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
  ewk_context_vibration_client_callbacks_set(ewk_context_,
                                             vibration_start_callback,
                                             vibration_stop_callback,
                                             NULL);



  // TODO(sngn.lee): Find the path of certificate file
  // ewk_context_certificate_file_set(ewk_context_, .... );

  // TODO(sngn.lee): find the proxy url
  // ewk_context_proxy_uri_set(ewk_context_, ... );

  // TODO(sngn.lee): set default from config.xml
  // locale_manager_->SetDefaultLocale(const  string & locale);

  // TODO(sngn.lee): Download interface
  // ewk_context_did_start_download_callback_set

  // TODO(sngn.lee): always enable "mediastream,record"
  // TODO(sngn.lee): always enable "encrypted,database"
  // TODO(sngn.lee): check csp element in config.xml and enable - "csp"
  // TODO(sngn.lee): Check Backround support and enable - "visibility,suspend"
  // TODO(sngn.lee): Check Backround support and enable - "background,music"
  // TODO(sngn.lee): Check setting rotation value and enable "rotation,lock"
  // TODO(sngn.lee): always enable "fullscreen"
  // TODO(sngn.lee): check "sound-mode":"exclusive" - in tizen:setting
  //                 and enable - "sound,mode"
  // TODO(sngn.lee): check "background-vibration":"enable" - in tizen:setting
  //                 and enable - "background,vibration"

  return true;
}

void WebApplication::Launch(std::unique_ptr<wrt::AppControl> appcontrol) {
  resource_manager_->set_app_control(appcontrol.get());
  WebView* view = new WebView(window_, ewk_context_);
  SetupWebView(view);

  // send widget info to injected bundle
  // TODO(wy80.choi): ewk_send_widget_info should be fixed to receive uuid of
  // application instead of widget_id.
  // Currently, uuid is passed as encoded_bundle argument temporarily.
  // ewk_send_widget_info(ewk_context_, 1,
  //                     elm_config_scale_get(),
  //                     elm_theme_get(NULL),
  //                    uuid_.c_str());

  // view->LoadUrl("file:///home/owner/apps_rw/33CFo0eFJe/"
  //               "33CFo0eFJe.annex/index.html");
  view->LoadUrl(resource_manager_->GetStartURL());

  view_stack_.push_front(view);
  window_->SetContent(view->evas_object());

  // TODO(sngn.lee): below code only debug code
  auto callback = [](void*, Evas*, Evas_Object* obj,
                     void*) -> void {
    int x, y, w, h;
    evas_object_geometry_get(obj, &x, &y, &w, &h);
    LoggerD("resize ! (%d, %d, %d, %d)\n", x, y, w, h);
  };
  evas_object_event_callback_add(view->evas_object(),
                                 EVAS_CALLBACK_RESIZE,
                                 callback, NULL);

  if (appcontrol->data(kDebugKey) == "true") {
    debug_mode_ = true;
    LaunchInspector(appcontrol.get());
  }

  // TODO(sngn.lee): check the below code location.
  // in Wearable, webkit can render contents before show window
  // but Mobile, webkit can't render contents before show window
  window_->Show();

  launched_ = true;
  received_appcontrol_ = std::move(appcontrol);
}

void WebApplication::AppControl(std::unique_ptr<wrt::AppControl> appcontrol) {
  // TODO(sngn.lee): Set the injected bundle into extension process
  resource_manager_->set_app_control(appcontrol.get());

  if (true) {
    // Reset to context
    ClearViewStack();
    WebView* view = new WebView(window_, ewk_context_);
    SetupWebView(view);

    view->LoadUrl(resource_manager_->GetStartURL());
    view_stack_.push_front(view);
    window_->SetContent(view->evas_object());
  } else {
    // Send Event
    SendAppControlEvent();
  }

  if (!debug_mode_ && appcontrol->data(kDebugKey) == "true") {
    debug_mode_ = true;
    LaunchInspector(appcontrol.get());
  }
  window_->Active();
  received_appcontrol_ = std::move(appcontrol);
}

void WebApplication::SendAppControlEvent() {
  auto it = view_stack_.begin();
  while (it != view_stack_.end()) {
    (*it)->EvalJavascript(kAppControlEventScript);
  }
}

void WebApplication::ClearViewStack() {
  window_->SetContent(NULL);
  auto it = view_stack_.begin();
  for ( ; it != view_stack_.end(); ++it) {
    (*it)->Suspend();
    delete *it;
  }
  view_stack_.clear();
}

void WebApplication::Resume() {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->SetVisibility(true);

  if (app_data_->setting_info() != NULL &&
      app_data_->setting_info()->background_support_enabled()) {
    return;
  }

  auto it = view_stack_.begin();
  for ( ; it != view_stack_.end(); ++it) {
    (*it)->Resume();
  }
}

void WebApplication::Suspend() {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->SetVisibility(false);

  if (app_data_->setting_info() != NULL &&
      app_data_->setting_info()->background_support_enabled()) {
    LoggerD("gone background (backgroud support enabed)");
    return;
  }

  auto it = view_stack_.begin();
  for ( ; it != view_stack_.end(); ++it) {
    (*it)->Suspend();
  }
}

void WebApplication::OnCreatedNewWebView(WebView* /*view*/, WebView* new_view) {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->SetVisibility(false);

  SetupWebView(new_view);
  view_stack_.push_front(new_view);
  window_->SetContent(new_view->evas_object());
}

void WebApplication::OnClosedWebView(WebView * view) {
  if (view_stack_.size() == 0)
    return;

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
    if (terminator_ != NULL) {
      terminator_();
    }
  } else if (current != view_stack_.front()) {
    view_stack_.front()->SetVisibility(true);
    window_->SetContent(view_stack_.front()->evas_object());
  }

  // Delete after the callback context(for ewk view) was not used
  ecore_idler_add([](void* view) {
      WebView* obj = static_cast<WebView*>(view);
      delete view;
      return EINA_FALSE;
    }, view);
}

void WebApplication::OnReceivedWrtMessage(
    WebView* /*view*/,
    Ewk_IPC_Wrt_Message_Data* /*message*/) {
  // TODO(wy80.choi) : Handle messages from injected bundle?
  // ex. SendRuntimeMessage to hide / exit application.
}

void WebApplication::OnOrientationLock(WebView* view,
                                       bool lock,
                                       int preferred_rotation) {
  if (view_stack_.size() == 0)
    return;

  // Only top-most view can set the orientation relate operation
  if (view_stack_.front() != view)
    return;

  auto orientaion_setting = app_data_->setting_info() != NULL ?
                            app_data_->setting_info()->screen_orientation() :
                            // TODO(sngn.lee): check default value
                            wgt::parse::SettingInfo::AUTO;
  if (orientaion_setting != wgt::parse::SettingInfo::AUTO) {
    return;
  }

  if ( lock ) {
    window_->SetRotationLock(preferred_rotation);
  } else {
    window_->SetAutoRotation();
  }
}

void WebApplication::OnHardwareKey(WebView* view, const std::string& keyname) {
  bool enabled = app_data_->setting_info() != NULL ?
                 app_data_->setting_info()->hwkey_enabled() :
                 true;
  if (enabled && kKeyNameBack == keyname) {
    view->EvalJavascript(kBackKeyEventScript);
  }
}

void WebApplication::OnLanguageChanged() {
  locale_manager_->UpdateSystemLocale();
  ewk_context_cache_clear(ewk_context_);
  auto it = view_stack_.begin();
  for ( ; it != view_stack_.end(); ++it) {
    (*it)->Reload();
  }
}

void WebApplication::OnConsoleMessage(const std::string& msg, int level) {
  static bool enabled = (getenv(kConsoleLogEnableKey) != NULL);
  if (debug_mode_ || enabled) {
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
    LOG_(LOG_ID_MAIN, dlog_level, kConsoleMessageLogTag, "%s", msg.c_str());
  }
}

void WebApplication::OnLowMemory() {
  ewk_context_cache_clear(ewk_context_);
  ewk_context_notify_low_memory(ewk_context_);
}

bool WebApplication::OnContextMenuDisabled(WebView* /*view*/) {
  return !(app_data_->setting_info() != NULL ?
           app_data_->setting_info()->context_menu_enabled() :
           true);
}

void WebApplication::OnLoadStart(WebView* /*view*/) {
  LoggerD("LoadStart");
}
void WebApplication::OnLoadFinished(WebView* /*view*/) {
  LoggerD("LoadFinished");
}
void WebApplication::OnRendered(WebView* /*view*/) {
  LoggerD("Rendered");
}

void WebApplication::LaunchInspector(wrt::AppControl* appcontrol) {
  unsigned int port =
    ewk_context_inspector_server_start(ewk_context_, 0);
  std::stringstream ss;
  ss << port;
  std::map<std::string, std::vector<std::string>> data;
  data[kPortKey] = { ss.str() };
  appcontrol->Reply(data);
}

void WebApplication::SetupWebView(WebView* view) {
  view->SetEventListener(this);
  // TODO(sngn.lee): set UserAgent to WebView
  // TODO(sngn.lee): set CSP
}

bool WebApplication::OnDidNavigation(WebView* view, const std::string& url) {
  // TODO(sngn.lee): scheme handling
  // except(file , http, https, app) pass to appcontrol and return false
  return true;
}


}  // namespace wrt
