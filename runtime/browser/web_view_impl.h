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

#ifndef XWALK_RUNTIME_BROWSER_WEB_VIEW_IMPL_H_
#define XWALK_RUNTIME_BROWSER_WEB_VIEW_IMPL_H_

#include <efl_extension.h>
#include <Elementary.h>
#include <ewk_chromium.h>

#include <map>
#include <string>

#include "common/url.h"
#include "runtime/browser/web_view.h"

namespace runtime {

class NativeWindow;
class WebViewImpl {
 public:
  WebViewImpl(WebView* view, NativeWindow* window, Ewk_Context* context);
  virtual ~WebViewImpl();

  void LoadUrl(const std::string& url, const std::string& mime = std::string());
  std::string GetUrl();

  void Suspend();
  void Resume();
  void Reload();
  void SetVisibility(bool show);
  bool EvalJavascript(const std::string& script);
  void SetAppInfo(const std::string& app_name, const std::string& version);
  bool SetUserAgent(const std::string& user_agent);
  void SetCSPRule(const std::string& rule, bool report_only);
  void SetDefaultEncoding(const std::string& encoding);

  void SetEventListener(WebView::EventListener* listener);
  Evas_Object* evas_object() const;
  std::string mime() const { return mime_; }

 private:
  void OnKeyEvent(Eext_Callback_Type key_type);
  void OnRotation(int degree);
  void Initialize();
  void Deinitialize();

  void InitKeyCallback();
  void InitLoaderCallback();
  void InitPolicyDecideCallback();
  void InitQuotaExceededCallback();
  void InitIPCMessageCallback();
  void InitConsoleMessageCallback();
  void InitCustomContextMenuCallback();
  void InitRotationCallback();
  void InitWindowCreateCallback();
  void InitFullscreenCallback();
  void InitNotificationPermissionCallback();
  void InitGeolocationPermissionCallback();
  void InitAuthenticationCallback();
  void InitCertificateAllowCallback();
  void InitPopupWaitCallback();
  void InitUsermediaCallback();

  NativeWindow* window_;
  Ewk_Context* context_;
  Evas_Object* ewk_view_;
  WebView::EventListener* listener_;
  int rotation_handler_id_;
  WebView* view_;
  std::map<const std::string, Evas_Smart_Cb> smart_callbacks_;
  bool fullscreen_;
  std::string mime_;
  Evas_Smart* evas_smart_class_;
  Ewk_View_Smart_Class ewk_smart_class_;
  bool internal_popup_opened_;
};
}  // namespace runtime

#endif  // XWALK_RUNTIME_BROWSER_WEB_VIEW_IMPL_H_
