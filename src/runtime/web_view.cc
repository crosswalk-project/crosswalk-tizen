// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/web_view.h"

#include <ewk_chromium.h>
#include <functional>

#include "runtime/native_window.h"

namespace wrt {

namespace {

// TODO(sngn.lee) : It should be declare in common header
const char* kKeyNameBack = "back";
const char* kKeyNameMenu = "menu";

static int ToWebRotation(int r) {
  switch (r) {
    case 90:
      return -90;
    case 270:
      return 90;
  }
  return r;
}

static int ToNativeRotation(int r) {
  switch (r) {
    case -90:
      return 90;
    case 90:
      return 270;
  }
  return r;
}

}  // namespace


WebView::WebView(NativeWindow* window, Ewk_Context* context)
    : window_(window),
      context_(context),
      ewk_view_(NULL),
      listener_(NULL) {
  Initialize();
}

WebView::~WebView() {
  window_->RemoveRotationHandler(rotation_handler_id_);
  evas_object_del(ewk_view_);
}

void WebView::LoadUrl(const std::string& url) {
  ewk_view_url_set(ewk_view_, url.c_str());
}

void WebView::Suspend() {
  // suspend webview
  ewk_view_suspend(ewk_view_);
}

void WebView::Resume() {
  // resume webview
  ewk_view_resume(ewk_view_);
}

void WebView::Reload() {
  ewk_view_reload(ewk_view_);
}

void WebView::SetVisibility(bool show) {
  ewk_view_visibility_set(ewk_view_, show ? EINA_TRUE : EINA_FALSE);
}


bool WebView::EvalJavascript(const std::string& script) {
  return ewk_view_script_execute(ewk_view_, script.c_str(), NULL, NULL);
}

void WebView::Initialize() {
  ewk_view_ = ewk_view_add_with_context(window_->evas_object(), context_);

  // TODO(sngn.lee): To be implemented - orientation lock


  auto key_callback = [](void* user_data,
                         Evas_Object* /*obj*/,
                         void* event_info) -> void {
    WebView* self = static_cast<WebView*>(user_data);
    Ea_Callback_Type key = static_cast<Ea_Callback_Type>(
                              reinterpret_cast<int>(event_info));
    self->OnKeyEvent(key);
  };
  ea_object_event_callback_add(ewk_view_,
                               EA_CALLBACK_BACK,
                               key_callback,
                               this);
  ea_object_event_callback_add(ewk_view_,
                               EA_CALLBACK_MORE,
                               key_callback,
                               this);


  // load statred callback
  auto loadstart_callback = [](void* user_data,
                               Evas_Object* /*obj*/,
                               void*) {
    WebView* self = static_cast<WebView*>(user_data);
    if (self->listener_)
      self->listener_->OnLoadStart(self);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "load,started",
                                 loadstart_callback,
                                 this);
  // load finished callback
  auto loadfinished_callback = [](void* user_data,
                                  Evas_Object*,
                                  void*) {
    WebView* self = static_cast<WebView*>(user_data);
    if (self->listener_)
      self->listener_->OnLoadFinished(self);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "load,finished",
                                 loadfinished_callback,
                                 this);

  // load progress callback
  auto loadprogress_callback = [](void* user_data,
                                  Evas_Object*,
                                  void* event_info) {
    WebView* self = static_cast<WebView*>(user_data);
    double* progress = static_cast<double*>(event_info);
    if (self->listener_)
      self->listener_->OnLoadProgress(self, *progress);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "load,progress",
                                 loadprogress_callback,
                                 this);
  // rendered callback
  auto rendered_callback = [](void* user_data,
                              Evas_Object*,
                              void*) {
    WebView* self = static_cast<WebView*>(user_data);
    if (self->listener_)
      self->listener_->OnRendered(self);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "frame,rendered",
                                 rendered_callback,
                                 this);

  // "policy,navigation,decide"
  auto navigation_decide_callback = [](void* user_data,
                                       Evas_Object*,
                                       void* event_info) {
    WebView* self = static_cast<WebView*>(user_data);
    Ewk_Policy_Decision* policy =
            static_cast<Ewk_Policy_Decision*>(event_info);
    const char* url = ewk_policy_decision_url_get(policy);

    if (self->listener_) {
      if (self->listener_->OnDidNavigation(self, url))
        ewk_policy_decision_use(policy);
      else
        ewk_policy_decision_ignore(policy);
    } else {
      ewk_policy_decision_use(policy);
    }
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "policy,navigation,decide",
                                 navigation_decide_callback,
                                 this);

  // policy,newwindow,decide
  auto newwindow_decide_callback = [](void* user_data,
                                      Evas_Object*,
                                      void* event_info) {
    WebView* self = static_cast<WebView*>(user_data);
    Ewk_Policy_Decision* policy =
            static_cast<Ewk_Policy_Decision*>(event_info);

    const char* url = ewk_policy_decision_url_get(policy);

    if (self->listener_) {
      if (self->listener_->OnDidNavigation(self, url) &&
         self->listener_->OnDidOpenWindow(self, url)) {
         ewk_policy_decision_use(policy);
      } else {
        ewk_policy_decision_ignore(policy);
      }
    } else {
      ewk_policy_decision_use(policy);
    }
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "policy,newwindow,decide",
                                 newwindow_decide_callback,
                                 this);

  // callback for database quota exceeded
  auto database_exceeded_callback = [](Evas_Object* view,
                                       Ewk_Security_Origin* origin,
                                       const char*,
                                       uint64_t,
                                       void*) -> Eina_Bool {
    std::string protocol(ewk_security_origin_protocol_get(origin));
    if (protocol == "file://" || protocol == "app://") {
      // Allow for local origin
      ewk_view_exceeded_database_quota_reply(view, EINA_TRUE);
    } else {
      // Deny for remote origin
      ewk_view_exceeded_database_quota_reply(view, EINA_FALSE);
    }
    return EINA_TRUE;
  };
  ewk_view_exceeded_database_quota_callback_set(
    ewk_view_,
    database_exceeded_callback,
    NULL);

  // callback for indexed database quota exceeded
  auto indexed_db_exceeded_callback = [](Evas_Object* view,
                                       Ewk_Security_Origin* origin,
                                       int64_t,
                                       void*) -> Eina_Bool {
    std::string protocol(ewk_security_origin_protocol_get(origin));
    if (protocol == "file://" || protocol == "app://") {
      // Allow for local origin
      ewk_view_exceeded_indexed_database_quota_reply(view, EINA_TRUE);
    } else {
      // Deny for remote origin
      ewk_view_exceeded_indexed_database_quota_reply(view, EINA_FALSE);
    }
    return EINA_TRUE;
  };
  ewk_view_exceeded_indexed_database_quota_callback_set(
    ewk_view_,
    indexed_db_exceeded_callback,
    NULL);

  // callback for localfile quota exceeded
  auto localfile_exceeded_callback = [](Evas_Object* view,
                                       Ewk_Security_Origin* origin,
                                       int64_t,
                                       void*) -> Eina_Bool {
    std::string protocol(ewk_security_origin_protocol_get(origin));
    if (protocol == "file://" || protocol == "app://") {
      // Allow for local origin
      ewk_view_exceeded_local_file_system_quota_reply(view, EINA_TRUE);
    } else {
      // Deny for remote origin
      ewk_view_exceeded_local_file_system_quota_reply(view, EINA_FALSE);
    }
    return EINA_TRUE;
  };
  ewk_view_exceeded_local_file_system_quota_callback_set(
    ewk_view_,
    localfile_exceeded_callback,
    NULL);

  // wrt,message
  auto wrt_message_callback = [](void* user_data,
                                 Evas_Object*,
                                 void* event_info) {
    WebView* self = static_cast<WebView*>(user_data);
    Ewk_IPC_Wrt_Message_Data* msg =
        static_cast<Ewk_IPC_Wrt_Message_Data*>(event_info);
    if (self->listener_)
      self->listener_->OnReceivedWrtMessage(self, msg);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "wrt,message",
                                 wrt_message_callback,
                                 this);

  // Orientation lock callback
  auto orientation_lock_callback = [](Evas_Object* o,
                                      Eina_Bool need_lock,
                                      int orientation,
                                      void* user_data) -> Eina_Bool {
    WebView* self = static_cast<WebView*>(user_data);
    if (self->listener_) {
      self->listener_->OnOrientationLock(self,
                                         need_lock,
                                         ToNativeRotation(orientation));
    }
    return EINA_TRUE;
  };
  ewk_view_orientation_lock_callback_set(ewk_view_,
                                         orientation_lock_callback,
                                         this);

  // rotation support
  ewk_view_orientation_send(ewk_view_, ToWebRotation(window_->rotation()));
  rotation_handler_id_ = window_->AddRotationHandler(
                                  std::bind(&WebView::OnRotation,
                                  this,
                                  std::placeholders::_1));
  // Show webview
  evas_object_show(ewk_view_);
}

std::string WebView::GetUrl() {
  return std::string(ewk_view_url_get(ewk_view_));
}

Evas_Object* WebView::evas_object() const {
  return ewk_view_;
}

void WebView::OnRotation(int degree) {
  ewk_view_orientation_send(ewk_view_, ToWebRotation(degree));
}

void WebView::OnKeyEvent(Ea_Callback_Type key_type) {
  std::string keyname;
  if (key_type == EA_CALLBACK_BACK) {
    if (EINA_TRUE == ewk_view_text_selection_clear(ewk_view_)) {
      return;
    }
    keyname = kKeyNameBack;
  } else if (key_type == EA_CALLBACK_MORE) {
    keyname = kKeyNameMenu;
  } else {
    return;
  }

  if (listener_)
    listener_->OnHardwareKey(this, keyname);
}


}  // namespace wrt

