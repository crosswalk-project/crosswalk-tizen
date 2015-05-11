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
#include "common/constants.h"
#include "common/command_line.h"
#include "common/string_utils.h"
#include "runtime/native_window.h"
#include "runtime/web_view.h"
#include "runtime/vibration_manager.h"
#include "common/app_control.h"
#include "common/locale_manager.h"
#include "common/application_data.h"
#include "common/resource_manager.h"
#include "runtime/app_db.h"

namespace wrt {

namespace {
// TODO(sngn.lee) : It should be declare in common header
const char* kKeyNameBack = "back";

const char* kConsoleLogEnableKey = "WRT_CONSOLE_LOG_ENABLE";
const char* kConsoleMessageLogTag = "ConsoleMessage";

const char* kDebugKey = "debug";
const char* kPortKey = "port";

// TODO(wy80.choi): consider 64bits system.
const char* kInjectedBundlePath = "/usr/lib/libwrt-injected-bundle.so";
const char* kDBusIntrospectionXML =
    "<node>"
    "  <interface name='org.tizen.wrt.Application'>"
    "    <method name='NotifyEPCreated'>"
    "      <arg name='status' type='s' direction='in'/>"
    "    </method>"
    "    <method name='GetRuntimeVariable'>"
    "      <arg name='key' type='s' direction='in' />"
    "      <arg name='value' type='s' direction='out' />"
    "    </method>"
    "  </interface>"
    "</node>";
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
const char* kFullscreenPrivilege = "http://tizen.org/privilege/fullscreen";
const char* kFullscreenFeature = "fullscreen";
const char* kNotificationPrivilege =
    "http://tizen.org/privilege/notification";
const char* kLocationPrivilege =
    "http://tizen.org/privilege/location";
const char* kStoragePrivilege =
    "http://tizen.org/privilege/unlimitedstorage";

const char* kVisibilitySuspendFeature = "visibility,suspend";
const char* kMediastreamRecordFeature = "mediastream,record";
const char* kEncryptedDatabaseFeature = "encrypted,database";
const char* kRotationLockFeature = "rotation,lock";
const char* kBackgroundMusicFeature = "background,music";
const char* kSoundModeFeature = "sound,mode";
const char* kBackgroundVibrationFeature = "background,vibration";
const char* kGeolocationPermissionPrefix = "__WRT_GEOPERM_";
const char* kNotificationPermissionPrefix = "__WRT_NOTIPERM_";
const char* kQuotaPermissionPrefix = "__WRT_QUOTAPERM_";



bool FindPrivilege(wrt::ApplicationData* app_data,
                   const std::string& privilege) {
  if (app_data->permissions_info().get() == NULL)
    return false;
  auto it = app_data->permissions_info()->GetAPIPermissions().begin();
  auto end = app_data->permissions_info()->GetAPIPermissions().end();
  for ( ; it != end; ++it) {
    if (*it == privilege)
      return true;
  }
  return false;
}

void ExecExtensionProcess(const std::string& uuid) {
  pid_t pid = -1;
  if ((pid = fork()) < 0) {
    LOGGER(ERROR) << "Failed to fork child process for extension process.";
  }
  if (pid == 0) {
    CommandLine* cmd = CommandLine::ForCurrentProcess();
    std::string switch_ext("--");
    switch_ext.append(kSwitchExtensionServer);
    execl(cmd->program().c_str(),
          cmd->program().c_str(), switch_ext.c_str(), uuid.c_str(), NULL);
  }
}

static void SendDownloadRequest(const std::string& url) {
  wrt::AppControl request;
  request.set_operation(APP_CONTROL_OPERATION_DOWNLOAD);
  request.set_uri(url);
  request.LaunchRequest();
}

}  // namespace

WebApplication::WebApplication(
    NativeWindow* window, std::unique_ptr<ApplicationData> app_data)
    : launched_(false),
      debug_mode_(false),
      ewk_context_(ewk_context_new_with_injected_bundle_path(
          kInjectedBundlePath)),
      window_(window),
      appid_(app_data->app_id()),
      app_uuid_(utils::GenerateUUID()),
      locale_manager_(new LocaleManager()),
      app_data_(std::move(app_data)),
      terminator_(NULL) {
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

  auto download_callback = [](const char* downloadUrl, void* data) {
    SendDownloadRequest(downloadUrl);
  };
  ewk_context_did_start_download_callback_set(ewk_context_,
                                              download_callback,
                                              this);

  if (FindPrivilege(app_data_.get(), kFullscreenPrivilege)) {
    ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                                kFullscreenFeature,
                                                true);
  }

  if (app_data_->setting_info() != NULL &&
      app_data_->setting_info()->background_support_enabled()) {
    ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                                kVisibilitySuspendFeature,
                                                true);
    ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                                kBackgroundMusicFeature,
                                                true);
  }
  ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                              kMediastreamRecordFeature,
                                              true);
  ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                              kEncryptedDatabaseFeature,
                                              true);
  if (app_data_->setting_info() != NULL &&
      app_data_->setting_info()->screen_orientation()
      == wgt::parse::SettingInfo::ScreenOrientation::AUTO) {
    ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                                kRotationLockFeature,
                                                true);
  }

  // TODO(sngn.lee): check "sound-mode":"exclusive" - in tizen:setting
  //                 and enable - "sound,mode"
  if (app_data_->setting_info() != NULL &&
      false/*"sound-mode":"exclusive"*/) {
    ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                                kSoundModeFeature,
                                                true);
  }

  // TODO(sngn.lee): check "background-vibration":"enable" - in tizen:setting
  //                 and enable - "background,vibration"
  if (app_data_->setting_info() != NULL &&
      false/*background-vibration":"enable"*/) {
    ewk_context_tizen_extensible_api_string_set(ewk_context_,
                                                kBackgroundVibrationFeature,
                                                true);
  }

  // TODO(sngn.lee): Find the path of certificate file
  // ewk_context_certificate_file_set(ewk_context_, .... );

  // TODO(sngn.lee): find the proxy url
  // ewk_context_proxy_uri_set(ewk_context_, ... );

  // TODO(sngn.lee): set default from config.xml
  // locale_manager_->SetDefaultLocale(const  string & locale);

  // TODO(sngn.lee): check csp element in config.xml and enable - "csp"

  return true;
}

void WebApplication::Launch(std::unique_ptr<wrt::AppControl> appcontrol) {
  // Start DBus Server for Runtime
  // TODO(wy80.choi): Should I add PeerCredentials checker?
  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  using std::placeholders::_4;
  dbus_server_.SetIntrospectionXML(kDBusIntrospectionXML);
  dbus_server_.SetMethodCallback(kDBusInterfaceNameForApplication,
    std::bind(&WebApplication::HandleDBusMethod, this, _1, _2, _3, _4));
  dbus_server_.Start(app_uuid_ +
                     "." + std::string(kDBusNameForApplication));

  // Execute ExtensionProcess
  ExecExtensionProcess(app_uuid_);

  // Setup View
  WebView* view = new WebView(window_, ewk_context_);
  SetupWebView(view);

  // send widget info to injected bundle
  // TODO(wy80.choi): ewk_send_widget_info should be fixed to receive uuid of
  // application instead of widget_id.
  // Currently, uuid is passed as encoded_bundle argument temporarily.
  // ewk_send_widget_info(ewk_context_, 1,
  //                      elm_config_scale_get(),
  //                      elm_theme_get(NULL),
  //                      app_uuid_.c_str());

  std::unique_ptr<ResourceManager::Resource> res =
    resource_manager_->GetStartResource(appcontrol.get());
  // TODO(wy80.choi): temporary comment for test, remove it later.
  // view->LoadUrl("file:///home/owner/apps_rw/33CFo0eFJe/"
  //               "33CFo0eFJe.annex/index.html");
  view->LoadUrl(res->uri());
  view_stack_.push_front(view);
  window_->SetContent(view->evas_object());

  // TODO(sngn.lee): below code only debug code
  auto callback = [](void*, Evas*, Evas_Object* obj,
                     void*) -> void {
    int x, y, w, h;
    evas_object_geometry_get(obj, &x, &y, &w, &h);
    LOGGER(DEBUG) << "resize ! ("
                  << x << ", " << y << ", " << w << ", " << h << ")";
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
  std::unique_ptr<ResourceManager::Resource> res =
    resource_manager_->GetStartResource(appcontrol.get());
  if (res->should_reset()) {
    // Reset to context
    ClearViewStack();
    WebView* view = new WebView(window_, ewk_context_);
    SetupWebView(view);

    view->LoadUrl(res->uri());
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
    LOGGER(DEBUG) << "gone background (backgroud support enabed)";
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
      delete obj;
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
                            wgt::parse::SettingInfo::ScreenOrientation::AUTO;
  if (orientaion_setting != wgt::parse::SettingInfo::ScreenOrientation::AUTO) {
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
  LOGGER(DEBUG) << "LoadStart";
}
void WebApplication::OnLoadFinished(WebView* /*view*/) {
  LOGGER(DEBUG) << "LoadFinished";
}
void WebApplication::OnRendered(WebView* /*view*/) {
  LOGGER(DEBUG) << "Rendered";
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

void WebApplication::OnNotificationPermissionRequest(
    WebView* view,
    const std::string& url,
    std::function<void(bool)> result_handler) {
  auto db = AppDB::GetInstance();
  std::string reminder = db->Get(kNotificationPermissionPrefix + url);
  if (reminder == "allowed") {
    result_handler(true);
  } else if (reminder == "denied") {
    result_handler(false);
  }

  // Local Domain: Grant permission if defined, otherwise Popup user prompt.
  // Remote Domain: Popup user prompt.
  if (utils::StartsWith(url, "file://") &&
      FindPrivilege(app_data_.get(), kNotificationPrivilege)) {
    result_handler(true);
    return;
  }

  // TODO(sngn.lee): create popup and show

  // TODO(sngn.lee): if alway check box was enabled save into db
  // if (false) {
  //  db->Set(kNotificationPermissionPrefix + url, result);
  // }
}

void WebApplication::OnGeolocationPermissionRequest(
    WebView* view,
    const std::string& url,
    std::function<void(bool)> result_handler) {
  auto db = AppDB::GetInstance();
  std::string reminder = db->Get(kGeolocationPermissionPrefix + url);
  if (reminder == "allowed") {
    result_handler(true);
  } else if (reminder == "denied") {
    result_handler(false);
  }

  // Local Domain: Grant permission if defined, otherwise block execution.
  // Remote Domain: Popup user prompt if defined, otherwise block execution.
  if (!FindPrivilege(app_data_.get(), kLocationPrivilege)) {
    result_handler(false);
    return;
  }

  if (utils::StartsWith(url, "file://")) {
    result_handler(true);
    return;
  }

  // TODO(sngn.lee): create popup and show
}


void WebApplication::OnQuotaExceed(
    WebView* view,
    const std::string& url,
    std::function<void(bool)> result_handler) {
  auto db = AppDB::GetInstance();
  std::string reminder = db->Get(kQuotaPermissionPrefix + url);
  if (reminder == "allowed") {
    result_handler(true);
  } else if (reminder == "denied") {
    result_handler(false);
  }

  // Local Domain: Grant permission if defined, otherwise Popup user prompt.
  // Remote Domain: Popup user prompt.
  if (utils::StartsWith(url, "file://") &&
      FindPrivilege(app_data_.get(), kStoragePrivilege)) {
    result_handler(true);
    return;
  }

  // TODO(sngn.lee): create popup and show
}

void WebApplication::OnAuthenticationRequest(
      WebView* view,
      const std::string& url,
      const std::string& message,
      std::function<void(bool submit,
                         const std::string& id,
                         const std::string& password)
                   > result_handler) {
  // TODO(sngn.lee): create popup and show
  result_handler(false, "", "");
}


void WebApplication::HandleDBusMethod(GDBusConnection* /*connection*/,
                                      const std::string& method_name,
                                      GVariant* parameters,
                                      GDBusMethodInvocation* invocation) {
  if (method_name == kMethodNotifyEPCreated) {
    LOGGER(DEBUG) << "Received 'NotifyEPCreated' from ExtensionServer.";
  } else if (method_name == kMethodGetRuntimeVariable) {
    gchar* key;
    std::string value;
    g_variant_get(parameters, "(&s)", &key);
    if (g_strcmp0(key, "runtime_name") == 0) {
      value = std::string("wrt");
    } else if (g_strcmp0(key, "app_id") == 0) {
      // TODO(wy80.choi): TEC requries double quotes, but webapi-plugins is not.
      value = "\"" + appid_ + "\"";
    } else if (g_strcmp0(key, "encoded_bundle") == 0) {
      value = received_appcontrol_->encoded_bundle();
    }
    g_dbus_method_invocation_return_value(
          invocation, g_variant_new("(s)", value.c_str()));
  }
}

}  // namespace wrt
