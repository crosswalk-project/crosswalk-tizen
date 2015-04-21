// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

  NativeWindow* window_;
  Ewk_Context* context_;
  Evas_Object* ewk_view_;
  WebView::EventListener* listener_;
  int rotation_handler_id_;
  WebView* view_;
  std::map<const std::string, Evas_Smart_Cb> smart_callbacks_;
};
}  // namespace wrt

#endif  // WRT_RUNTIME_WEB_VIEW_IMPL_H_
