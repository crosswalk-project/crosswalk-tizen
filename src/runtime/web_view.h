// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_WEB_VIEW_H_
#define WRT_RUNTIME_WEB_VIEW_H_
#include <Elementary.h>
#include <efl_assist.h>
#include <string>

class Ewk_Context;

namespace wrt {
class NativeWindow;

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
    virtual bool OnDidOpenWindow(WebView* /*view*/,
                                 const std::string& /*url*/) { return true; }
    virtual bool OnDidNavigation(WebView* /*view*/,
                                 const std::string& /*url*/) { return true; }
    virtual void OnHardwareKey(WebView* /*void*/,
                               const std::string& /*keyname*/) {}
  };

  WebView(wrt::NativeWindow* window, Ewk_Context* context);
  virtual ~WebView();

  void LoadUrl(const std::string& url);
  std::string GetUrl();

  void Suspend();
  void Resume();
  void Reload();
  void AlwaysRun(bool run);
  bool EvalJavascript(const std::string& script);

  void SetEventListener(EventListener* listener);
  Evas_Object* evas_object() const;

 private:
  void OnKeyEvent(Ea_Callback_Type key_type);
  void OnRotation(int degree);
  void Initialize();
  NativeWindow* window_;
  Ewk_Context* context_;
  Evas_Object* ewk_view_;
  EventListener* listener_;
  bool always_run_;
  int rotation_handler_id_;
};

}  // namespace wrt

#endif  // WRT_RUNTIME_WEB_VIEW_H_
