// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_WEB_APPLICATION_H_
#define WRT_RUNTIME_WEB_APPLICATION_H_

#include <string>
#include <list>
#include "runtime/web_view.h"

class Ewk_Context;

namespace wrt {
class NativeWindow;

class WebApplication : public WebView::EventListener {
 public:
  explicit WebApplication(const std::string& appid);
  virtual ~WebApplication();

  void AppControl();
  void Launch();
  void Resume();
  void Suspend();

  bool Initialize(NativeWindow* window);
  std::string GetDataPath() const;
  bool initialized() const { return initialized_; }

  virtual void OnCreatedNewWebView(WebView* view, WebView* new_view);
  virtual void OnClosedWebView(WebView * view);

 private:
  void ClearViewStack();
  void SendAppControlEvent();

  bool initialized_;
  std::string appid_;
  Ewk_Context* ewk_context_;
  NativeWindow* window_;
  std::list<WebView*> view_stack_;
};

}  // namespace wrt

#endif  // WRT_RUNTIME_WEB_APPLICATION_H_
