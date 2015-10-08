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

class WebApplication : public WebView::EventListener {
 public:
  WebApplication(NativeWindow* window,
                 std::unique_ptr<common::ApplicationData> app_data);
  virtual ~WebApplication();

  void AppControl(std::unique_ptr<common::AppControl> appcontrol);
  void Launch(std::unique_ptr<common::AppControl> appcontrol);
  void Resume();
  void Suspend();
  void Terminate();

  std::string data_path() const { return app_data_path_; }
  void set_terminator(std::function<void(void)> terminator)
      { terminator_ = terminator; }
  bool launched() const { return launched_; }

  virtual void OnCreatedNewWebView(WebView* view, WebView* new_view);
  virtual void OnClosedWebView(WebView * view);
  virtual void OnReceivedWrtMessage(
      WebView* view,
      Ewk_IPC_Wrt_Message_Data* msg);
  virtual void OnOrientationLock(
      WebView* view,
      bool lock,
      NativeWindow::ScreenOrientation preferred_rotation);
  virtual void OnHardwareKey(WebView* view, const std::string& keyname);
  virtual void OnConsoleMessage(const std::string& msg, int level);
  virtual void OnLoadStart(WebView* view);
  virtual void OnLoadFinished(WebView* view);
  virtual void OnRendered(WebView* view);
  virtual void OnLanguageChanged();
  virtual void OnLowMemory();
  virtual bool OnContextMenuDisabled(WebView* view);
  virtual bool OnDidNavigation(WebView* view, const std::string& url);
  virtual void OnNotificationPermissionRequest(
      WebView* view,
      const std::string& url,
      std::function<void(bool)> result_handler);
  virtual void OnGeolocationPermissionRequest(
      WebView* view,
      const std::string& url,
      std::function<void(bool)> result_handler);
  virtual void OnQuotaExceed(
      WebView* view,
      const std::string& url,
      std::function<void(bool)> result_handler);
  virtual void OnAuthenticationRequest(
      WebView* view,
      const std::string& url,
      const std::string& message,
      std::function<void(bool submit,
                         const std::string& id,
                         const std::string& password)
                   > result_handler);
  virtual void OnCertificateAllowRequest(
      WebView* view,
      const std::string& url,
      const std::string& pem,
      std::function<void(bool allow)> result_handler);
  virtual void OnUsermediaPermissionRequest(
      WebView* view,
      const std::string& url,
      std::function<void(bool)> result_handler);

 private:
  bool Initialize();

  void ClearViewStack();
  void SendAppControlEvent();
  void LaunchInspector(common::AppControl* appcontrol);
  void SetupWebView(WebView* view);

  bool launched_;
  bool debug_mode_;
  Ewk_Context* ewk_context_;
  NativeWindow* window_;
  std::string appid_;
  std::string app_data_path_;
  std::list<WebView*> view_stack_;
  std::unique_ptr<common::LocaleManager> locale_manager_;
  std::unique_ptr<common::ApplicationData> app_data_;
  std::unique_ptr<common::ResourceManager> resource_manager_;
  std::function<void(void)> terminator_;
  int security_model_version_;
  std::string csp_rule_;
  std::string csp_report_rule_;
};

}  // namespace runtime

#endif  // XWALK_RUNTIME_BROWSER_WEB_APPLICATION_H_
