// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_WEB_APPLICATION_H_
#define WRT_RUNTIME_WEB_APPLICATION_H_

#include <string>
#include <list>
#include <memory>

#include "runtime/web_view.h"
#include "extension/extension_server.h"

class Ewk_Context;

namespace wrt {
class NativeWindow;
class AppControl;
class ApplicationData;
class LocaleManager;

class WebApplication : public WebView::EventListener {
 public:
  explicit WebApplication(const std::string& appid);
  explicit WebApplication(std::unique_ptr<ApplicationData> app_data);
  virtual ~WebApplication();

  void AppControl(std::unique_ptr<wrt::AppControl> appcontrol);
  void Launch(std::unique_ptr<wrt::AppControl> appcontrol);
  void Resume();
  void Suspend();

  bool Initialize(NativeWindow* window);
  std::string data_path() const { return app_data_path_; }
  bool initialized() const { return initialized_; }

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

 private:
  void ClearViewStack();
  void SendAppControlEvent();
  void LaunchInspector(wrt::AppControl* appcontrol);

  bool initialized_;
  std::string appid_;
  Ewk_Context* ewk_context_;
  NativeWindow* window_;
  ExtensionServer* extension_server_;
  std::list<WebView*> view_stack_;
  std::string app_data_path_;
  std::unique_ptr<LocaleManager> locale_manager_;
  std::unique_ptr<ApplicationData> app_data_;
  bool debug_mode_;
};

}  // namespace wrt

#endif  // WRT_RUNTIME_WEB_APPLICATION_H_
