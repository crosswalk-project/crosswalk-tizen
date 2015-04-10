// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/web_application.h"

#include <cert-service.h>
#include <app.h>
#include <ewk_chromium.h>
#include <algorithm>
#include <memory>

#include "runtime/native_window.h"
#include "runtime/command_line.h"
#include "runtime/web_view.h"
#include "runtime/vibration_manager.h"

namespace {

  // TODO(sngn.lee) : It should be declare in common header
  const char* kKeyNameBack = "back";

  const char* kAppControlEventScript = \
        "var __event = document.createEvent(\"CustomEvent\");\n"
        "__event.initCustomEvent(\"appcontrol\", true, true);\n"
        "document.dispatchEvent(__event);\n"
        "\n"
        "for (var i=0; i < window.frames.length; i++)\n"
        "{ window.frames[i].document.dispatchEvent(__event); }";
  const char* kBackKeyEventScript = \
        "var __event = document.createEvent(\"CustomEvent\");\n"
        "__event.initCustomEvent(\"tizenhwkey\", true, true);\n"
        "__event.keyName = \"back\";\n"
        "document.dispatchEvent(__event);\n"
        "\n"
        "for (var i=0; i < window.frames.length; i++)\n"
        "{ window.frames[i].document.dispatchEvent(__event); }";

}  // namespace

namespace wrt {

WebApplication::WebApplication(const std::string& appid)
    : initialized_(false),
      appid_(appid), ewk_context_(ewk_context_new()) {
  std::unique_ptr<char, decltype(std::free)*>
    path {app_get_data_path(), std::free};
  app_data_path_ = path.get();
}

WebApplication::~WebApplication() {
  ewk_context_delete(ewk_context_);
}

bool WebApplication::Initialize(NativeWindow* window) {
  window_ = window;

  // ewk setting
  ewk_context_cache_model_set(ewk_context_, EWK_CACHE_MODEL_DOCUMENT_BROWSER);

  // cookie
  auto cookie_manager = ewk_context_cookie_manager_get(ewk_context_);
  ewk_cookie_manager_accept_policy_set(cookie_manager,
                                       EWK_COOKIE_ACCEPT_POLICY_ALWAYS);

  // set persistent storage path
  std::string cookie_path = data_path() + ".cookie";
  ewk_cookie_manager_persistent_storage_set(
                                      cookie_manager, cookie_path.c_str(),
                                      EWK_COOKIE_PERSISTENT_STORAGE_SQLITE);

  // vibration callback
  auto vibration_start_callback = [](uint64_t ms, void*) {
    platform::VibrationManager::GetInstance()->Start(static_cast<int>(ms));
  };
  auto vibration_stop_callback = [](void* /*user_data*/) {
    platform::VibrationManager::GetInstance()->Stop();
  };
  ewk_context_vibration_client_callbacks_set(ewk_context_,
                                             vibration_start_callback,
                                             vibration_stop_callback,
                                             NULL);

  // Set certificate path
  char* cert_path = cert_svc_get_certificate_crt_file_path();
  if (cert_path != NULL) {
    ewk_context_certificate_file_set(ewk_context_, cert_path);
  }

  // TODO(sngn.lee): find the proxy url
  // ewk_context_proxy_uri_set(ewk_context_, ... );
  return true;
}

void WebApplication::Launch() {
  initialized_ = true;
  WebView* view = new WebView(window_, ewk_context_);

  // TODO(sngn.lee): Get the start file
  view->LoadUrl("index.html");
  view_stack_.push_front(view);
  window_->SetContent(view->evas_object());

  // TODO(sngn.lee): below code only debug code
  auto callback = [](void* data, Evas* e, Evas_Object* obj, void* eventInfo) -> void {
    int x,y,w,h;
    evas_object_geometry_get(obj, &x,&y,&w,&h);
    fprintf(stderr,"resize ! (%d, %d, %d, %d)\n", x,y,w,h);
  };
  evas_object_event_callback_add(view->evas_object(), EVAS_CALLBACK_RESIZE, callback, NULL );

  // TODO(sngn.lee): check the below code location.
  // in Wearable, webkit can render contents before show window
  // but Mobile, webkit can't render contents before show window
  window_->Show();
}

void WebApplication::AppControl() {
  // TODO(sngn.lee): find the app control url and the reset options

  // TODO(sngn.lee): Set the injected bundle into extension process


  if (true) {
    // Reset to context
    ClearViewStack();
    WebView* view = new WebView(window_, ewk_context_);
    view->LoadUrl("file:///index.html");
    view_stack_.push_front(view);
    window_->SetContent(view->evas_object());
  } else {
    // Send Event
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
  window_->SetContent(NULL);
  auto it = view_stack_.begin();
  while (it != view_stack_.end()) {
    (*it)->Suspend();
    delete *it;
  }
  view_stack_.clear();
}

void WebApplication::Resume() {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->SetVisibility(true);

  // TODO(sngn.lee) : should be check the background support option
  // if background suuport options was on, skip below code

  auto it = view_stack_.begin();
  for ( ; it != view_stack_.end(); ++it) {
    (*it)->Resume();
  }
}

void WebApplication::Suspend() {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->SetVisibility(false);

  // TODO(sngn.lee) : should be check the background support option
  // if background suuport options was on, skip below code
  auto it = view_stack_.begin();
  for ( ; it != view_stack_.end(); ++it) {
    (*it)->Suspend();
  }
}

void WebApplication::OnCreatedNewWebView(WebView* view, WebView* new_view) {
  if (view_stack_.size() > 0 && view_stack_.front() != NULL)
    view_stack_.front()->SetVisibility(false);

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
    if (found != view_stack_.end()) {
      view_stack_.erase(found);
    }
  }

  if (view_stack_.size() == 0) {
    // TODO(sngn.lee): terminate the webapp
  } else if (current != view_stack_.front()) {
    view_stack_.front()->SetVisibility(true);
    window_->SetContent(view_stack_.front()->evas_object());
  }

  delete view;
}

void WebApplication::OnRendered(WebView* view) {
}


void WebApplication::OnReceivedWrtMessage(
    WebView* view,
    const Ewk_IPC_Wrt_Message_Data& message) {
  // TODO(wy80.choi): To be implemented
}

void WebApplication::OnOrientationLock(WebView* view,
                                       bool lock,
                                       int preferred_rotation) {
  if (view_stack_.size() == 0)
    return;

  // Only top-most view can set the orientation relate operation
  if (view_stack_.front() != view)
    return;

  // TODO(sngn.lee): check the orientaion setting
  // if allow the auto orientation
  // if (is not allow orientation) {
  //   return;
  // }
  if ( lock ) {
    window_->SetRotationLock(preferred_rotation);
  } else {
    window_->SetAutoRotation();
  }
}

void WebApplication::OnHardwareKey(WebView* view, const std::string& keyname) {
  // TODO(sngn.lee): Check the hw key event was enabled
  if (true && kKeyNameBack == keyname) {
    view->EvalJavascript(kBackKeyEventScript);
  }
}

}  // namespace wrt
