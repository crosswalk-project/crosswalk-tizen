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

#ifndef XWALK_RUNTIME_BROWSER_WEB_VIEW_H_
#define XWALK_RUNTIME_BROWSER_WEB_VIEW_H_

#include <Elementary.h>
#include <ewk_chromium.h>
#include <functional>
#include <string>

#include "runtime/browser/native_window.h"

class Ewk_Context;

namespace runtime {
class WebViewImpl;

enum class RotaryEventType {
  CLOCKWISE,  // Rotary is rotated clockwise direction
  COUNTER_CLOCKWISE  // Rotary is rotated counter clockwise direction
};

class SoftKeyboardChangeEventValue {
 public:
  std::string state;
  int width;
  int height;
};

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
        NativeWindow::ScreenOrientation /*preferred_rotation*/) {}
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
    virtual void OnUsermediaPermissionRequest(
        WebView* /*view*/,
        const std::string& /*url*/,
        std::function<void(bool)> /*result_handler*/) {}
    virtual void OnSoftKeyboardChangeEvent(
        WebView* /*view*/,
        SoftKeyboardChangeEventValue /*softkeyboard_value*/) {}
#ifdef ROTARY_EVENT_FEATURE_SUPPORT
    virtual void OnRotaryEvent(
        WebView* /*view*/,
        RotaryEventType /*type*/) {}
#endif  // ROTARY_EVENT_FEATURE_SUPPORT
#ifdef MANUAL_ROTATE_FEATURE_SUPPORT
    virtual void OnRotatePrepared(WebView* /*view*/) {}
#endif // MANUAL_ROTATE_FEATURE_SUPPORT
  };

  WebView(NativeWindow* window, Ewk_Context* context);
  virtual ~WebView();

  void LoadUrl(const std::string& url, const std::string& mime = std::string());
  std::string GetUrl();

  void Suspend();
  void Resume();
  void Reload();
  bool Backward();
  void SetVisibility(bool show);
  bool EvalJavascript(const std::string& script);
  void SetAppInfo(const std::string& app_name, const std::string& version);
  bool SetUserAgent(const std::string& user_agent);
  void SetCSPRule(const std::string& rule, bool report_only);
  void SetDefaultEncoding(const std::string& encoding);
#ifdef PROFILE_WEARABLE
  void SetBGColor(int r, int g, int b, int a);
#endif

  void SetEventListener(EventListener* listener);
  Evas_Object* evas_object() const;

 private:
  WebViewImpl* impl_;
};

}  // namespace runtime

#endif  // XWALK_RUNTIME_BROWSER_WEB_VIEW_H_
