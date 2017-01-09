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

#ifndef XWALK_RUNTIME_BROWSER_WEB_APPLICATION_H_
#define XWALK_RUNTIME_BROWSER_WEB_APPLICATION_H_

#include <functional>
#include <list>
#include <memory>
#include <string>

#include "runtime/browser/web_view.h"

class Ewk_Context;

namespace common {
class AppControl;
class ApplicationData;
class LocaleManager;
class ResourceManager;
}  // namespace common

namespace runtime {
class NativeWindow;
class SplashScreen;

class WebApplication : public WebView::EventListener {
 public:
  WebApplication(NativeWindow* window,
                 common::ApplicationData* app_data);
  WebApplication(NativeWindow* window,
                 common::ApplicationData* app_data,
                 Ewk_Context* context);
  virtual ~WebApplication();

  void AppControl(std::unique_ptr<common::AppControl> appcontrol);
  void Launch(std::unique_ptr<common::AppControl> appcontrol);
  void Resume();
  void Suspend();
  void Terminate();

  void ClosePageFromOnTerminate();
  std::string data_path() const { return app_data_path_; }
  void set_terminator(std::function<void(void)> terminator) {
    terminator_ = terminator;
  }
  bool launched() const { return launched_; }
  std::list<WebView*> view_stack() const { return view_stack_; }

  virtual void OnCreatedNewWebView(WebView* view, WebView* new_view);
  virtual void OnClosedWebView(WebView* view);
  virtual void OnReceivedWrtMessage(WebView* view,
                                    Ewk_IPC_Wrt_Message_Data* msg);
  virtual void OnOrientationLock(
      WebView* view, bool lock,
      NativeWindow::ScreenOrientation preferred_rotation);
  virtual void OnHardwareKey(WebView* view, const std::string& keyname);
  virtual void OnConsoleMessage(const std::string& msg, int level);
  virtual void OnLoadStart(WebView* view);
  virtual void OnLoadFinished(WebView* view);
  virtual void OnRendered(WebView* view);
  virtual void OnLanguageChanged();
  virtual void OnLowMemory();
  virtual void OnTimeTick(long time);
  virtual void OnAmbientTick(long time);
  virtual void OnAmbientChanged(bool ambient_mode);
  virtual bool OnContextMenuDisabled(WebView* view);
  virtual bool OnDidNavigation(WebView* view, const std::string& url);
  virtual void OnNotificationPermissionRequest(
      WebView* view, const std::string& url,
      std::function<void(bool)> result_handler);
  virtual void OnGeolocationPermissionRequest(
      WebView* view, const std::string& url,
      std::function<void(bool)> result_handler);
  virtual void OnQuotaExceed(WebView* view, const std::string& url,
                             std::function<void(bool)> result_handler);
  virtual void OnAuthenticationRequest(
      WebView* view, const std::string& url, const std::string& message,
      std::function<void(bool submit, const std::string& id,
                         const std::string& password)> result_handler);
  virtual void OnCertificateAllowRequest(
      WebView* view, const std::string& url, const std::string& pem,
      std::function<void(bool allow)> result_handler);
  virtual void OnUsermediaPermissionRequest(
      WebView* view, const std::string& url,
      std::function<void(bool)> result_handler);
  virtual void OnSoftKeyboardChangeEvent(
      WebView* view, SoftKeyboardChangeEventValue softkeyboard_value);
#ifdef ROTARY_EVENT_FEATURE_SUPPORT
  virtual void OnRotaryEvent(
      WebView* view, RotaryEventType type);
#endif  // ROTARY_EVENT_FEATURE_SUPPORT
#ifdef MANUAL_ROTATE_FEATURE_SUPPORT
  virtual void OnRotatePrepared(WebView* view);
#endif // MANUAL_ROTATE_FEATURE_SUPPORT

 private:
  bool Initialize();

  void ClearViewStack();
  void SendAppControlEvent();
  void LaunchInspector(common::AppControl* appcontrol);
  void SetupWebView(WebView* view);
  void SetupWebViewCompatibilitySettings(WebView* view);
  void RemoveWebViewFromStack(WebView* view);

  void SetupTizenVersion();
  bool tizenWebKitCompatibilityEnabled() const;
  struct {
      unsigned m_major;
      unsigned m_minor;
      unsigned m_release;
      bool tizenWebKitCompatibilityEnabled() const { return (m_major && m_major < 3); }
  } m_tizenCompatibilitySettings;

  bool launched_;
  bool debug_mode_;
  bool verbose_mode_;
  bool lang_changed_mode_;
  Ewk_Context* ewk_context_;
  bool has_ownership_of_ewk_context_;
  NativeWindow* window_;
  std::string appid_;
  std::string app_data_path_;
  common::ApplicationData* app_data_;
  std::list<WebView*> view_stack_;
  std::unique_ptr<SplashScreen> splash_screen_;
  std::unique_ptr<common::LocaleManager> locale_manager_;
  std::unique_ptr<common::ResourceManager> resource_manager_;
  std::function<void(void)> terminator_;
  int security_model_version_;
  std::string csp_rule_;
  std::string csp_report_rule_;
};

}  // namespace runtime

#endif  // XWALK_RUNTIME_BROWSER_WEB_APPLICATION_H_
