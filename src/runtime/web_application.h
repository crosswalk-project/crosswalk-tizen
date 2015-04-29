// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_WEB_APPLICATION_H_
#define WRT_RUNTIME_WEB_APPLICATION_H_

#include <string>
#include <list>
#include <memory>
#include <functional>

#include "common/dbus_server.h"
#include "runtime/web_view.h"

class Ewk_Context;

namespace wrt {
class NativeWindow;
class AppControl;
class ApplicationData;
class LocaleManager;
class ResourceManager;

class WebApplication : public WebView::EventListener {
 public:
  WebApplication(NativeWindow* window,
                 std::unique_ptr<ApplicationData> app_data);
  virtual ~WebApplication();

  void AppControl(std::unique_ptr<wrt::AppControl> appcontrol);
  void Launch(std::unique_ptr<wrt::AppControl> appcontrol);
  void Resume();
  void Suspend();

  std::string data_path() const { return app_data_path_; }
  void set_terminator(std::function<void(void)> terminator)
      { terminator_ = terminator; }
  bool launched() const { return launched_; }

  virtual void OnCreatedNewWebView(WebView* view, WebView* new_view);
  virtual void OnClosedWebView(WebView * view);
  virtual void OnReceivedWrtMessage(WebView* view,
                                    Ewk_IPC_Wrt_Message_Data* message);
  virtual void OnOrientationLock(WebView* view,
                                 bool lock,
                                 int preferred_rotation);
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

 private:
  bool Initialize();

  void ClearViewStack();
  void SendAppControlEvent();
  void LaunchInspector(wrt::AppControl* appcontrol);
  void SetupWebView(WebView* view);

  void HandleDBusMethod(GDBusConnection* connection,
                        const std::string& method_name,
                        GVariant* parameters,
                        GDBusMethodInvocation* invocation);

  bool launched_;
  bool debug_mode_;
  Ewk_Context* ewk_context_;
  NativeWindow* window_;
  DBusServer dbus_server_;
  std::string appid_;
  std::string app_data_path_;
  std::string app_uuid_;
  std::list<WebView*> view_stack_;
  std::unique_ptr<LocaleManager> locale_manager_;
  std::unique_ptr<ApplicationData> app_data_;
  std::unique_ptr<ResourceManager> resource_manager_;
  std::unique_ptr<wrt::AppControl> received_appcontrol_;
  std::function<void(void)> terminator_;
};

}  // namespace wrt

#endif  // WRT_RUNTIME_WEB_APPLICATION_H_
