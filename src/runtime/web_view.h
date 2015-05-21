// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_WEB_VIEW_H_
#define WRT_RUNTIME_WEB_VIEW_H_

#include <Elementary.h>
#include <ewk_ipc_message.h>
#include <string>
#include <functional>

class Ewk_Context;

namespace wrt {
class NativeWindow;
class WebViewImpl;

class WebView {
 public:
  class EventListener {
   public:
    virtual void OnLoadStart(WebView* /*view*/) {}
    virtual void OnLoadProgress(WebView* /*view*/, double /*persent*/ ) {}
    virtual void OnLoadFinished(WebView* /*view*/) {}
    virtual void OnRendered(WebView* /*view*/) {}
    virtual void OnCreatedNewWebView(WebView* /*view*/,
                                     WebView* /*new_view*/) {}
    virtual void OnClosedWebView(WebView* /*view*/) {}
    virtual void OnCrashed(WebView* /*view*/) {}
    virtual bool OnDidOpenWindow(
        WebView* /*view*/, const std::string& /*url*/) { return true; }
    virtual bool OnDidNavigation(
        WebView* /*view*/, const std::string& /*url*/) { return true; }
    virtual void OnHardwareKey(
        WebView* /*view*/, const std::string& /*keyname*/) {}
    virtual void OnReceivedWrtMessage(
        WebView* /*view*/, Ewk_IPC_Wrt_Message_Data* /*msg*/) {}
    virtual void OnOrientationLock(
        WebView* /*view*/,
        bool /*lock*/,
        int /*preferred_rotation*/) {}
    virtual void OnConsoleMessage(const std::string& /*msg*/, int /*level*/) {}
    virtual bool OnContextMenuDisabled(WebView* /*view*/) { return false; }

    virtual void OnNotificationPermissionRequest(
        WebView* /*view*/,
        const std::string& /*url*/,
        std::function<void(bool)> /*result_handler*/) {}
    virtual void OnGeolocationPermissionRequest(
        WebView* /*view*/,
        const std::string& /*url*/,
        std::function<void(bool)> /*result_handler*/) {}
    virtual void OnQuotaExceed(
        WebView* /*view*/,
        const std::string& /*url*/,
        std::function<void(bool)> /*result_handler*/) {}
    virtual void OnAuthenticationRequest(
        WebView* /*view*/,
        const std::string& /*url*/,
        const std::string& /*message*/,
        std::function<void(bool /*submit*/,
                           const std::string& /*id*/,
                           const std::string& /*password*/)
                     > /*result_handler*/) {}
    virtual void OnCertificateAllowRequest(
        WebView* /*view*/,
        const std::string& /*url*/,
        const std::string& /*pem*/,
        std::function<void(bool allow)> result_handler) {
      result_handler(false);
    }
  };

  WebView(wrt::NativeWindow* window, Ewk_Context* context);
  virtual ~WebView();

  void LoadUrl(const std::string& url);
  std::string GetUrl();

  void Suspend();
  void Resume();
  void Reload();
  void SetVisibility(bool show);
  bool EvalJavascript(const std::string& script);
  void SetAppInfo(const std::string& app_name, const std::string& version);
  bool SetUserAgent(const std::string& user_agent);

  void SetEventListener(EventListener* listener);
  Evas_Object* evas_object() const;

 private:
  WebViewImpl* impl_;
};

}  // namespace wrt

#endif  // WRT_RUNTIME_WEB_VIEW_H_
