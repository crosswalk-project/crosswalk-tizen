// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/web_view_impl.h"

#include <ewk_chromium.h>
#include <functional>
#include <sstream>

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

WebViewImpl::WebViewImpl(WebView* view,
                         NativeWindow* window,
                         Ewk_Context* context)
    : window_(window),
      context_(context),
      ewk_view_(NULL),
      listener_(NULL),
      view_(view) {
  Initialize();
}

WebViewImpl::~WebViewImpl() {
  Deinitialize();
  evas_object_del(ewk_view_);
}

void WebViewImpl::LoadUrl(const std::string& url) {
  ewk_view_url_set(ewk_view_, url.c_str());
}

void WebViewImpl::Suspend() {
  // suspend webview
  ewk_view_suspend(ewk_view_);
}

void WebViewImpl::Resume() {
  // resume webview
  ewk_view_resume(ewk_view_);
}

void WebViewImpl::Reload() {
  ewk_view_reload(ewk_view_);
}

void WebViewImpl::SetVisibility(bool show) {
  ewk_view_visibility_set(ewk_view_, show ? EINA_TRUE : EINA_FALSE);
}


bool WebViewImpl::EvalJavascript(const std::string& script) {
  return ewk_view_script_execute(ewk_view_, script.c_str(), NULL, NULL);
}

void WebViewImpl::Initialize() {
  ewk_view_ = ewk_view_add_with_context(window_->evas_object(), context_);

  InitKeyCallback();
  InitLoaderCallback();
  InitPolicyDecideCallback();
  InitQuotaExceededCallback();
  InitIPCMessageCallback();
  InitOrientaionLockCallback();
  InitConsoleMessageCallback();
  InitCustomContextMenuCallback();
  InitRotationCallback();
  InitWindowCreateCallback();

  // TODO(sngn.lee): "request,certificate,confirm" certification popup
  // TODO(sngn.lee): ewk_view_notification_permission_callback_set
  // TODO(sngn.lee): "notification,show"
  // TODO(sngn.lee): "notification,cancel"
  // TODO(sngn.lee): "create,window"
  // TODO(sngn.lee): "close,window"
  // TODO(sngn.lee): "fullscreen,enterfullscreen"
  // TODO(sngn.lee): "fullscreen,exitfullscreen"
  // TODO(sngn.lee): "protocolhandler,registration,requested"
  //                  custom protocol handler
  // TODO(sngn.lee): ewk_view_geolocation_permission_callback_set
  // TODO(sngn.lee): ewk_view_user_media_permission_callback_set

  // Show webview
  evas_object_show(ewk_view_);
}

void WebViewImpl::Deinitialize() {
  auto it = smart_callbacks_.begin();
  for ( ; it != smart_callbacks_.end(); ++it) {
    evas_object_smart_callback_del(
        ewk_view_,
        it->first.c_str(),
        it->second);
  }
  ea_object_event_callback_del(ewk_view_,
                               EA_CALLBACK_BACK,
                               smart_callbacks_["key_callback"]);
  ewk_view_exceeded_database_quota_callback_set(
      ewk_view_,
      NULL,
      NULL);
  ewk_view_exceeded_indexed_database_quota_callback_set(
      ewk_view_,
      NULL,
      NULL);
  ewk_view_exceeded_local_file_system_quota_callback_set(
      ewk_view_,
      NULL,
      NULL);
  ewk_view_orientation_lock_callback_set(
      ewk_view_,
      NULL,
      NULL);
  window_->RemoveRotationHandler(rotation_handler_id_);
}

void WebViewImpl::InitKeyCallback() {
  auto key_callback = [](void* user_data,
                         Evas_Object* /*obj*/,
                         void* event_info) -> void {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    Ea_Callback_Type key = static_cast<Ea_Callback_Type>(
                              reinterpret_cast<int>(event_info));
    self->OnKeyEvent(key);
  };
  ea_object_event_callback_add(ewk_view_,
                               EA_CALLBACK_BACK,
                               key_callback,
                               view_);
  ea_object_event_callback_add(ewk_view_,
                               EA_CALLBACK_MORE,
                               key_callback,
                               view_);
  smart_callbacks_["key_callback"] = key_callback;
}

void WebViewImpl::InitLoaderCallback() {
  // load statred callback
  auto loadstart_callback = [](void* user_data,
                               Evas_Object* /*obj*/,
                               void*) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (self->listener_)
      self->listener_->OnLoadStart(self->view_);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "load,started",
                                 loadstart_callback,
                                 this);
  // load finished callback
  auto loadfinished_callback = [](void* user_data,
                                  Evas_Object*,
                                  void*) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (self->listener_)
      self->listener_->OnLoadFinished(self->view_);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "load,finished",
                                 loadfinished_callback,
                                 this);

  // load progress callback
  auto loadprogress_callback = [](void* user_data,
                                  Evas_Object*,
                                  void* event_info) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    double* progress = static_cast<double*>(event_info);
    if (self->listener_)
      self->listener_->OnLoadProgress(self->view_, *progress);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "load,progress",
                                 loadprogress_callback,
                                 this);
  // rendered callback
  auto rendered_callback = [](void* user_data,
                              Evas_Object*,
                              void*) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (self->listener_)
      self->listener_->OnRendered(self->view_);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "frame,rendered",
                                 rendered_callback,
                                 this);
  smart_callbacks_["load,started"] = loadstart_callback;
  smart_callbacks_["load,finished"] = loadfinished_callback;
  smart_callbacks_["load,progress"] = loadprogress_callback;
  smart_callbacks_["frame,rendered"] = rendered_callback;
}

void WebViewImpl::InitPolicyDecideCallback() {
  // "policy,navigation,decide"
  auto navigation_decide_callback = [](void* user_data,
                                       Evas_Object*,
                                       void* event_info) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    Ewk_Policy_Decision* policy =
            static_cast<Ewk_Policy_Decision*>(event_info);
    const char* url = ewk_policy_decision_url_get(policy);

    if (self->listener_) {
      if (self->listener_->OnDidNavigation(self->view_, url))
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
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    Ewk_Policy_Decision* policy =
            static_cast<Ewk_Policy_Decision*>(event_info);

    const char* url = ewk_policy_decision_url_get(policy);

    if (self->listener_) {
      if (self->listener_->OnDidNavigation(self->view_, url) &&
         self->listener_->OnDidOpenWindow(self->view_, url)) {
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
  smart_callbacks_["policy,navigation,decide"] = navigation_decide_callback;
  smart_callbacks_["policy,newwindow,decide"] = newwindow_decide_callback;
}

void WebViewImpl::InitQuotaExceededCallback() {
  // TODO(sngn.lee): Need callback interface - OnQutaExceed
  // check http://tizen.org/privilege/unlimitedstorage

  // callback for database quota exceeded
  auto database_exceeded_callback = [](Evas_Object* view,
                                       Ewk_Security_Origin* origin,
                                       const char*,
                                       uint64_t,
                                       void*) -> Eina_Bool {
    std::string protocol(ewk_security_origin_protocol_get(origin));
    if (protocol == "file" || protocol == "app") {
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
}

void WebViewImpl::InitIPCMessageCallback() {
  // wrt,message
  auto wrt_message_callback = [](void* user_data,
                                 Evas_Object*,
                                 void* event_info) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    Ewk_IPC_Wrt_Message_Data* msg =
        static_cast<Ewk_IPC_Wrt_Message_Data*>(event_info);
    if (self->listener_)
      self->listener_->OnReceivedWrtMessage(self->view_, msg);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "wrt,message",
                                 wrt_message_callback,
                                 this);
  smart_callbacks_["wrt,message"] = wrt_message_callback;
}

void WebViewImpl::InitOrientaionLockCallback() {
  // Orientation lock callback
  auto orientation_lock_callback = [](Evas_Object*,
                                      Eina_Bool need_lock,
                                      int orientation,
                                      void* user_data) -> Eina_Bool {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (self->listener_) {
      self->listener_->OnOrientationLock(self->view_,
                                         need_lock,
                                         ToNativeRotation(orientation));
    }
    return EINA_TRUE;
  };
  ewk_view_orientation_lock_callback_set(ewk_view_,
                                         orientation_lock_callback,
                                         this);
}

void WebViewImpl::InitConsoleMessageCallback() {
  // console log
  auto console_message_callback = [](void* user_data,
                                 Evas_Object*,
                                 void* event_info) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (!self->listener_) {
      return;
    }
    Ewk_Console_Message* msg = static_cast<Ewk_Console_Message*>(event_info);
    unsigned int line_number = ewk_console_message_line_get(msg);

    std::stringstream buf;
    if (line_number) {
        buf << ewk_console_message_source_get(msg) << ":";
        buf << line_number << ":";
    }
    buf << ewk_console_message_text_get(msg);
    int level = ewk_console_message_level_get(msg);
    self->listener_->OnConsoleMessage(buf.str(), level);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "console,message",
                                 console_message_callback,
                                 this);
  smart_callbacks_["console,message"] = console_message_callback;
}

void WebViewImpl::InitCustomContextMenuCallback() {
  auto custom_context_menu_callback = [](void* user_data,
                                         Evas_Object*,
                                         void* event_info) {
    Ewk_Context_Menu* contextmenu = static_cast<Ewk_Context_Menu*>(event_info);
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    bool disabled = false;
    if (self->listener_ &&
        self->listener_->OnContextMenuDisabled(self->view_)) {
      disabled = true;
    }
    int cnt = ewk_context_menu_item_count(contextmenu);
    for (unsigned idx = cnt-1; idx > 0; --idx) {
      auto* item = ewk_context_menu_nth_item_get(contextmenu, idx);
      Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
      switch (tag) {
        case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW:
        case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW:
        case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_FRAME_IN_NEW_WINDOW:
        case EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB:
        case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK:
          ewk_context_menu_item_remove(contextmenu, item);
          break;
        default:
          if (disabled)
            ewk_context_menu_item_remove(contextmenu, item);
      }
    }
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "contextmenu,customize",
                                 custom_context_menu_callback,
                                 this);
  smart_callbacks_["contextmenu,customize"] = custom_context_menu_callback;
}

void WebViewImpl::InitRotationCallback() {
  // rotation support
  ewk_view_orientation_send(ewk_view_, ToWebRotation(window_->rotation()));
  rotation_handler_id_ = window_->AddRotationHandler(
                                  std::bind(&WebViewImpl::OnRotation,
                                  this,
                                  std::placeholders::_1));
}

void WebViewImpl::InitWindowCreateCallback() {
  auto create_callback = [](void* user_data,
                            Evas_Object*,
                            void* event_info) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (!self->listener_) {
      return;
    }
    WebView* new_view = new WebView(self->window_, self->context_);
    self->listener_->OnCreatedNewWebView(self->view_, new_view);
  };

  auto close_callback = [](void* user_data,
                            Evas_Object*,
                            void*) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (!self->listener_) {
      return;
    }
    self->listener_->OnClosedWebView(self->view_);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "create,window",
                                 create_callback,
                                 this);
  evas_object_smart_callback_add(ewk_view_,
                                 "close,window",
                                 close_callback,
                                 this);

  smart_callbacks_["create,window"] = create_callback;
  smart_callbacks_["close,window"] = close_callback;
}

std::string WebViewImpl::GetUrl() {
  return std::string(ewk_view_url_get(ewk_view_));
}

Evas_Object* WebViewImpl::evas_object() const {
  return ewk_view_;
}

void WebViewImpl::OnRotation(int degree) {
  ewk_view_orientation_send(ewk_view_, ToWebRotation(degree));
}

void WebViewImpl::OnKeyEvent(Ea_Callback_Type key_type) {
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
    listener_->OnHardwareKey(view_, keyname);
}

void WebViewImpl::SetEventListener(WebView::EventListener* listener) {
  listener_ = listener;
}

void WebViewImpl::SetAppInfo(const std::string& app_name,
                             const std::string& version) {
  std::string ua = app_name + "/" + version;
  ewk_view_application_name_for_user_agent_set(ewk_view_, ua.c_str());
}
void WebViewImpl::SetUserAgent(const std::string& user_agent) {
  ewk_view_user_agent_set(ewk_view_, user_agent.c_str());
}

}  // namespace wrt
