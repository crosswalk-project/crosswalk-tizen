// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "web_application.h"

#include <algorithm>
#include <ewk_chromium.h>

#include "native_window.h"
#include "command_line.h"
#include "web_view.h"


namespace {
  const char* kAppControlEventScript = \
        "var __event = document.createEvent(\"CustomEvent\");\n"
        "__event.initCustomEvent(\"appcontrol\", true, true);\n"
        "document.dispatchEvent(__event);\n"
        "\n"
        "for (var i=0; i < window.frames.length; i++)\n"
        "{ window.frames[i].document.dispatchEvent(__event); }";
}

namespace wrt {

WebApplication::WebApplication(const std::string& appid)
    : initialized_(false),
      appid_(appid), ewk_context_(ewk_context_new()) {
}

WebApplication::~WebApplication() {
  ewk_context_delete(ewk_context_);
}

bool WebApplication::Initialize(NativeWindow* window) {
  window_ = window;

  char* chromium_arg_options[] = {
    CommandLine::ForCurrentProcess()->argv()[0],
    const_cast<char*>("--enable-file-cookies"),
    const_cast<char*>("--allow-file-access-from-files"),
    const_cast<char*>("--allow-universal-access-from-files")
  };
  const int chromium_arg_cnt = sizeof(chromium_arg_options) / sizeof(chromium_arg_options[0]);
  ewk_set_arguments(chromium_arg_cnt, chromium_arg_options);

  //ewk setting
  ewk_context_cache_model_set(ewk_context_, EWK_CACHE_MODEL_DOCUMENT_BROWSER);

  //cookie
  auto cookie_manager = ewk_context_cookie_manager_get(ewk_context_);
  ewk_cookie_manager_accept_policy_set(cookie_manager, EWK_COOKIE_ACCEPT_POLICY_ALWAYS);

  //set persistent storage path
  std::string cookie_path = GetDataPath() + ".cookie";
  ewk_cookie_manager_persistent_storage_set(cookie_manager, cookie_path.c_str(), EWK_COOKIE_PERSISTENT_STORAGE_SQLITE);

  //TODO. Find the path of certificate file
  //ewk_context_certificate_file_set(ewk_context_, .... );

  //TODO. find the proxy url
  //ewk_context_proxy_uri_set(ewk_context_, ... );
  return true;
}

void WebApplication::Launch() {
  initialized_ = true;
  WebView* view = new WebView(window_, ewk_context_);

  //TODO. Get the start file
  view->LoadUrl("index.html");
  view_stack_.push_front(view);
  window_->SetContent(view->evas_object());
}

void WebApplication::AppControl() {

  //TODO. find the app control url and the reset options

  //TODO. Set the injected bundle into extension process


  if (true) {
    //Reset to context
    ClearViewStack();
    WebView* view = new WebView(window_, ewk_context_);
    view->LoadUrl("index.html");
    view_stack_.push_front(view);
    window_->SetContent(view->evas_object());
  } else {
    //Send Event
    SendAppControlEvent();
  }
}

void WebApplication::SendAppControlEvent() {
  auto it = view_stack_.begin();
  while (it != view_stack_.end()) {
    (*it)->EvalJavascript(kAppControlEventScript);
  }
}

void WebApplication::ClearViewStack() {
  auto it = view_stack_.begin();
  while (it != view_stack_.end()) {
    (*it)->Suspend();
    delete *it;
  }
  view_stack_.clear();
}

void WebApplication::Resume() {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->Resume();

}

void WebApplication::Suspend() {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->Suspend();
}


void WebApplication::OnCreatedNewWebView(WebView* view, WebView* new_view) {
  view->Suspend();
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->Suspend();

  //TODO. check the background support option
  //new_view.AlwaysRun(false);
  view_stack_.push_front(new_view);
  window_->SetContent(new_view->evas_object());
}

void WebApplication::OnClosedWebView(WebView * view) {
  if (view_stack_.size() == 0)
    return;

  WebView* current = view_stack_.front();
  if (current == view) {
    view_stack_.pop_front();
  } else {
    auto found = std::find(view_stack_.begin(), view_stack_.end(), view);
    if(found != view_stack_.end()) {
      view_stack_.erase(found);
    }
  }

  delete view;

  if (view_stack_.size() == 0) {
    //TODO. terminate the webapp
  } else if (current != view_stack_.front()){
    window_->SetContent(view_stack_.front()->evas_object());
  }
}



std::string WebApplication::GetDataPath() const {
  //TODO. To be Implements
  return std::string("./");
}

} // namespace wrt
