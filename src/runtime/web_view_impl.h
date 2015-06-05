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


#ifndef WRT_RUNTIME_WEB_VIEW_IMPL_H_
#define WRT_RUNTIME_WEB_VIEW_IMPL_H_


#include <Elementary.h>
#include <efl_assist.h>
#include <string>
#include <map>

#include "runtime/web_view.h"

class Ewk_Context;

namespace wrt {
class NativeWindow;
class WebViewImpl {
 public:
  WebViewImpl(WebView* view, wrt::NativeWindow* window, Ewk_Context* context);
  virtual ~WebViewImpl();

  void LoadUrl(const std::string& url);
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

 private:
  void OnKeyEvent(Ea_Callback_Type key_type);
  void OnRotation(int degree);
  void Initialize();
  void Deinitialize();

  void InitKeyCallback();
  void InitLoaderCallback();
  void InitPolicyDecideCallback();
  void InitQuotaExceededCallback();
  void InitIPCMessageCallback();
  void InitOrientaionLockCallback();
  void InitConsoleMessageCallback();
  void InitCustomContextMenuCallback();
  void InitRotationCallback();
  void InitWindowCreateCallback();
  void InitFullscreenCallback();
  void InitNotificationPermissionCallback();
  void InitGeolocationPermissionCallback();
  void InitAuthenticationCallback();
  void InitCertificateAllowCallback();

  NativeWindow* window_;
  Ewk_Context* context_;
  Evas_Object* ewk_view_;
  WebView::EventListener* listener_;
  int rotation_handler_id_;
  WebView* view_;
  std::map<const std::string, Evas_Smart_Cb> smart_callbacks_;
  bool fullscreen_;
};
}  // namespace wrt

#endif  // WRT_RUNTIME_WEB_VIEW_IMPL_H_
