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


#include "runtime/browser/web_view_impl.h"

#include <ewk_chromium.h>
#include <functional>
#include <sstream>

#include "common/file_utils.h"
#include "common/logger.h"
#include "common/profiler.h"
#include "runtime/browser/native_window.h"

namespace runtime {

namespace {

const char* kKeyNameBack = "back";
const char* kKeyNameMenu = "menu";
const char* kDefaultEncoding = "UTF-8";
const char* kSmartClassUserDataKey = "__SC_USERDATA__";

static int ToWebRotation(int r) {
  switch (r) {
    case 90:
      return -90;
    case 270:
      return 90;
  }
  return r;
}

static NativeWindow::ScreenOrientation ToNativeRotation(int r) {
  if (r ==
      (EWK_SCREEN_ORIENTATION_PORTRAIT_PRIMARY
       | EWK_SCREEN_ORIENTATION_PORTRAIT_SECONDARY
       | EWK_SCREEN_ORIENTATION_LANDSCAPE_PRIMARY
       | EWK_SCREEN_ORIENTATION_LANDSCAPE_SECONDARY)) {
    return NativeWindow::ScreenOrientation::ANY;
  } else if (r ==
      (EWK_SCREEN_ORIENTATION_PORTRAIT_PRIMARY
       | EWK_SCREEN_ORIENTATION_LANDSCAPE_PRIMARY)) {
    return NativeWindow::ScreenOrientation::NATURAL;
  } else if (r & EWK_SCREEN_ORIENTATION_PORTRAIT_PRIMARY) {
    return NativeWindow::ScreenOrientation::PORTRAIT_PRIMARY;
  } else if (r & EWK_SCREEN_ORIENTATION_PORTRAIT_SECONDARY) {
    return NativeWindow::ScreenOrientation::PORTRAIT_SECONDARY;
  } else if (r & EWK_SCREEN_ORIENTATION_LANDSCAPE_PRIMARY) {
    return NativeWindow::ScreenOrientation::LANDSCAPE_PRIMARY;
  } else {
    return NativeWindow::ScreenOrientation::LANDSCAPE_SECONDARY;
  }
}

}  // namespace

WebViewImpl::WebViewImpl(WebView* view,
                         NativeWindow* window,
                         Ewk_Context* context)
    : window_(window),
      context_(context),
      ewk_view_(NULL),
      listener_(NULL),
      rotation_handler_id_(0),
      view_(view),
      fullscreen_(false),
      evas_smart_class_(NULL),
      internal_popup_opened_(false),
      ime_width_(0),
      ime_height_(0) {
  Initialize();
}

WebViewImpl::~WebViewImpl() {
  if (internal_popup_opened_) {
    ewk_view_javascript_alert_reply(ewk_view_);
  }
  Deinitialize();
  evas_object_del(ewk_view_);
  if (evas_smart_class_ != NULL)
    evas_smart_free(evas_smart_class_);
}

void WebViewImpl::LoadUrl(const std::string& url, const std::string& mime) {
  SCOPE_PROFILE();
  if (!mime.empty()) {
    mime_set_cb_ = [url, mime]
                   (const char* request_url, const char* request_mime,
                    char** new_mime, void* data) {
      WebViewImpl* view = static_cast<WebViewImpl*>(data);
      if (view != nullptr &&
          common::utils::BaseName(url) ==
          common::utils::BaseName(request_url)) {
        *new_mime = strdup(mime.c_str());
        LOGGER(DEBUG) << "ewk's new_mime: " << *new_mime;
        return EINA_TRUE;
      }
      return EINA_FALSE;
    };
    auto mime_override_cb = [](const char* url, const char* mime,
                                 char** new_mime, void* data) -> Eina_Bool {
      WebViewImpl* view = static_cast<WebViewImpl*>(data);
      return view->mime_set_cb_(url, mime, new_mime, data);
    };
    ewk_context_mime_override_callback_set(context_, mime_override_cb, this);
  }
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

bool WebViewImpl::Backward() {
  if (ewk_view_back_possible(ewk_view_)) {
    ewk_view_back(ewk_view_);
    return EINA_TRUE;
  }
  return EINA_FALSE;
}

void WebViewImpl::SetVisibility(bool show) {
  ewk_view_page_visibility_state_set(ewk_view_,
                                     show ? EWK_PAGE_VISIBILITY_STATE_VISIBLE :
                                            EWK_PAGE_VISIBILITY_STATE_HIDDEN,
                                     EINA_FALSE);
}


bool WebViewImpl::EvalJavascript(const std::string& script) {
  return ewk_view_script_execute(ewk_view_, script.c_str(), NULL, NULL);
}

void WebViewImpl::Initialize() {
  ewk_smart_class_ = EWK_VIEW_SMART_CLASS_INIT_NAME_VERSION("WebView");
  ewk_view_smart_class_set(&ewk_smart_class_);
  ewk_smart_class_.orientation_lock = [](Ewk_View_Smart_Data *sd,
                                         int orientation) {
    WebViewImpl* self = static_cast<WebViewImpl*>(
        evas_object_data_get(sd->self, kSmartClassUserDataKey));
    if (self == NULL || self->listener_ == NULL)
      return EINA_FALSE;
    self->listener_->OnOrientationLock(self->view_,
                                       true,
                                       ToNativeRotation(orientation));
    return EINA_TRUE;
  };

  ewk_smart_class_.orientation_unlock = [](Ewk_View_Smart_Data *sd) {
    WebViewImpl* self = static_cast<WebViewImpl*>(
        evas_object_data_get(sd->self, kSmartClassUserDataKey));
    if (self == NULL || self->listener_ == NULL)
      return;
    self->listener_->OnOrientationLock(
        self->view_,
        false,
        NativeWindow::ScreenOrientation::PORTRAIT_PRIMARY);
  };

  if (evas_smart_class_ != NULL)
    evas_smart_free(evas_smart_class_);
  evas_smart_class_ = evas_smart_class_new(&ewk_smart_class_.sc);
  if (evas_smart_class_ == NULL) {
    LOGGER(ERROR) << "Can't create evas smart class";
    return;
  }

  Ewk_Page_Group* page_group = ewk_page_group_create("");
  ewk_view_ = ewk_view_smart_add(evas_object_evas_get(window_->evas_object()),
                                 evas_smart_class_,
                                 context_,
                                 page_group);
  evas_object_data_set(ewk_view_, kSmartClassUserDataKey, this);

  InitKeyCallback();
  InitLoaderCallback();
  InitPolicyDecideCallback();
  InitQuotaExceededCallback();
  InitIPCMessageCallback();
  InitConsoleMessageCallback();
  InitCustomContextMenuCallback();
  InitRotationCallback();
  InitWindowCreateCallback();
  InitFullscreenCallback();
  InitNotificationPermissionCallback();
  InitGeolocationPermissionCallback();
  InitAuthenticationCallback();
  InitCertificateAllowCallback();
  InitPopupWaitCallback();
  InitUsermediaCallback();
  InitEditorClientImeCallback();
#ifdef ROTARY_EVENT_FEATURE_SUPPORT
  InitRotaryEventCallback();
#endif  // ROTARY_EVENT_FEATURE_SUPPORT

  Ewk_Settings* settings = ewk_view_settings_get(ewk_view_);
  ewk_settings_scripts_can_open_windows_set(settings, EINA_TRUE);
  ewk_settings_default_text_encoding_name_set(settings, kDefaultEncoding);

  // TODO(sngn.lee): "protocolhandler,registration,requested"
  //                  custom protocol handler

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
  eext_object_event_callback_del(ewk_view_,
                               EEXT_CALLBACK_BACK,
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
  ewk_view_notification_permission_callback_set(
      ewk_view_,
      NULL,
      NULL);
  ewk_view_geolocation_permission_callback_set(
      ewk_view_,
      NULL,
      NULL);
  ewk_view_user_media_permission_callback_set(
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
    Eext_Callback_Type key = static_cast<Eext_Callback_Type>(
      reinterpret_cast<long long>(event_info));  // NOLINT
    self->OnKeyEvent(key);
  };
  eext_object_event_callback_add(ewk_view_,
                               EEXT_CALLBACK_BACK,
                               key_callback,
                               this);
  eext_object_event_callback_add(ewk_view_,
                               EEXT_CALLBACK_MORE,
                               key_callback,
                               this);
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

#ifdef MANUAL_ROTATE_FEATURE_SUPPORT
  // rotate prepared callback
  auto rotateprepared_callback = [](void* user_data,
                              Evas_Object*,
                              void*) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (self->listener_)
      self->listener_->OnRotatePrepared(self->view_);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "rotate,prepared",
                                 rotateprepared_callback,
                                 this);
  smart_callbacks_["rotate,prepared"] = rotateprepared_callback;
#endif  // MANUAL_ROTATE_FEATURE_SUPPORT
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
                                       unsigned long long, // NOLINT
                                       void* user_data) -> Eina_Bool {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (self == NULL || self->listener_ == NULL)
      return EINA_TRUE;

    auto result_handler = [view](bool result) {
      LOGGER(DEBUG) << "database quota Permission Result : " << result;
      ewk_view_exceeded_database_quota_reply(view, result);
    };
    std::stringstream url;
    url << ewk_security_origin_protocol_get(origin)
        << "://"
        << ewk_security_origin_host_get(origin)
        << ":"
        << ewk_security_origin_port_get(origin);
    self->listener_->OnQuotaExceed(
        self->view_,
        url.str(),
        result_handler);
    return EINA_TRUE;
  };
  ewk_view_exceeded_database_quota_callback_set(
    ewk_view_,
    database_exceeded_callback,
    this);

  // callback for indexed database quota exceeded
  auto indexed_db_exceeded_callback = [](Evas_Object* view,
                                       Ewk_Security_Origin* origin,
                                       long long, // NOLINT
                                       void* user_data) -> Eina_Bool {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (self == NULL || self->listener_ == NULL)
      return EINA_TRUE;

    auto result_handler = [view](bool result) {
      LOGGER(DEBUG) << "indexed db quota Permission Result : " << result;
      ewk_view_exceeded_indexed_database_quota_reply(view, result);
    };
    std::stringstream url;
    url << ewk_security_origin_protocol_get(origin)
        << "://"
        << ewk_security_origin_host_get(origin)
        << ":"
        << ewk_security_origin_port_get(origin);
    self->listener_->OnQuotaExceed(
        self->view_,
        url.str(),
        result_handler);
    return EINA_TRUE;
  };
  ewk_view_exceeded_indexed_database_quota_callback_set(
    ewk_view_,
    indexed_db_exceeded_callback,
    this);

  // callback for localfile quota exceeded
  auto localfile_exceeded_callback = [](Evas_Object* view,
                                       Ewk_Security_Origin* origin,
                                       long long, // NOLINT
                                       void* user_data) -> Eina_Bool {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (self == NULL || self->listener_ == NULL)
      return EINA_TRUE;

    auto result_handler = [view](bool result) {
      LOGGER(DEBUG) << "local file quota Permission Result : " << result;
      ewk_view_exceeded_local_file_system_quota_reply(view, result);
    };
    std::stringstream url;
    url << ewk_security_origin_protocol_get(origin)
        << "://"
        << ewk_security_origin_host_get(origin)
        << ":"
        << ewk_security_origin_port_get(origin);
    self->listener_->OnQuotaExceed(
        self->view_,
        url.str(),
        result_handler);
    return EINA_TRUE;
  };
  ewk_view_exceeded_local_file_system_quota_callback_set(
    ewk_view_,
    localfile_exceeded_callback,
    this);
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
        buf << ewk_console_message_source_get(msg)
            << ":" << line_number << ": ";
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
    for (int idx = cnt-1; idx >= 0; --idx) {
      auto* item = ewk_context_menu_nth_item_get(contextmenu, idx);
      Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);
      switch (tag) {
        case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW:
        case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW:
        case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_FRAME_IN_NEW_WINDOW:
        case EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB:
        case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK:
        case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK:
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
    *(static_cast<Evas_Object **>(event_info)) = new_view->evas_object();
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

void WebViewImpl::InitFullscreenCallback() {
  auto enter_callback = [](void* user_data,
                            Evas_Object*,
                            void* /*event_info*/) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    self->fullscreen_ = true;
    self->window_->FullScreen(true);
  };
  auto exit_callback =  [](void* user_data,
                            Evas_Object*,
                            void* /*event_info*/) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    self->fullscreen_ = false;
    self->window_->FullScreen(false);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "fullscreen,enterfullscreen",
                                 enter_callback,
                                 this);
  evas_object_smart_callback_add(ewk_view_,
                                 "fullscreen,exitfullscreen",
                                 exit_callback,
                                 this);
  smart_callbacks_["fullscreen,enterfullscreen"] = enter_callback;
  smart_callbacks_["fullscreen,exitfullscreen"] = exit_callback;
}

void WebViewImpl::InitNotificationPermissionCallback() {
  auto request_callback = [](Evas_Object*,
                             Ewk_Notification_Permission_Request* request,
                             void* user_data) {
    LOGGER(DEBUG) << "Notification Permission Request";
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (!self->listener_) {
      ewk_notification_permission_reply(request, EINA_FALSE);
      return EINA_TRUE;
    }

    ewk_notification_permission_request_suspend(request);
    auto result_handler = [request](bool result) {
      LOGGER(DEBUG) << "Notification Permission Result : %d" << result;
      ewk_notification_permission_reply(request, result);
    };
    const Ewk_Security_Origin* ewk_origin =
        ewk_notification_permission_request_origin_get(request);

    std::stringstream url;
    url << ewk_security_origin_protocol_get(ewk_origin)
        << "://"
        << ewk_security_origin_host_get(ewk_origin)
        << ":"
        << ewk_security_origin_port_get(ewk_origin);
    self->listener_->OnNotificationPermissionRequest(
        self->view_,
        url.str(),
        result_handler);
    return EINA_TRUE;
  };
  ewk_view_notification_permission_callback_set(ewk_view_,
                                                request_callback,
                                                this);
}

void WebViewImpl::InitGeolocationPermissionCallback() {
  auto permission_callback = [](
      Evas_Object*,
      Ewk_Geolocation_Permission_Request* request,
      void* user_data) {
    LOGGER(DEBUG) << "Geolocation Permission Request";
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (self == NULL || self->listener_ == NULL) {
      ewk_geolocation_permission_reply(request, EINA_FALSE);
      return EINA_TRUE;
    }
    ewk_geolocation_permission_request_suspend(request);

    const Ewk_Security_Origin* ewk_origin =
        ewk_geolocation_permission_request_origin_get(request);
    auto result_handler = [request](bool result) {
      LOGGER(DEBUG) << "Geolocation Permission Result : " << result;
      ewk_geolocation_permission_reply(request, result);
    };

    std::stringstream url;
    url << ewk_security_origin_protocol_get(ewk_origin)
        << "://"
        << ewk_security_origin_host_get(ewk_origin)
        << ":"
        << ewk_security_origin_port_get(ewk_origin);

    self->listener_->OnGeolocationPermissionRequest(
        self->view_,
        url.str(),
        result_handler);
    return EINA_TRUE;
  };
  ewk_view_geolocation_permission_callback_set(ewk_view_,
                                               permission_callback,
                                               this);
}

void WebViewImpl::InitAuthenticationCallback() {
  auto auth_callback = [](void* user_data,
                          Evas_Object*,
                          void* event_info) {
    LOGGER(DEBUG) << "Authentication Request";
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    Ewk_Auth_Challenge* auth_challenge =
        static_cast<Ewk_Auth_Challenge*>(event_info);

    if (self == NULL || self->listener_ == NULL) {
      ewk_auth_challenge_credential_cancel(auth_challenge);
      return;
    }
    auto result_handler = [auth_challenge](bool submit,
                                    const std::string& id,
                                    const std::string& password) {
      LOGGER(DEBUG) << "Authentication Result : submit = " << submit;
      if (!submit) {
        ewk_auth_challenge_credential_cancel(auth_challenge);
        return;
      }
      ewk_auth_challenge_credential_use(auth_challenge,
                                        id.c_str(),
                                        password.c_str());
    };
    ewk_auth_challenge_suspend(auth_challenge);
    const char* message =
        ewk_auth_challenge_realm_get(auth_challenge);
    std::string url = self->GetUrl();
    self->listener_->OnAuthenticationRequest(self->view_,
                                             url,
                                             message,
                                             result_handler);
  };
  // "authentication,challenge"
  evas_object_smart_callback_add(ewk_view_,
                                 "authentication,challenge",
                                 auth_callback,
                                 this);
  smart_callbacks_["authentication,challenge"] = auth_callback;
}

void WebViewImpl::InitCertificateAllowCallback() {
  auto certi_callback = [](void* user_data,
                           Evas_Object*,
                           void* event_info) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    Ewk_Certificate_Policy_Decision* policy =
      static_cast<Ewk_Certificate_Policy_Decision*>(event_info);

    if (self == NULL || self->listener_ == NULL) {
      ewk_certificate_policy_decision_allowed_set(policy, EINA_FALSE);
      return;
    }

    ewk_certificate_policy_decision_suspend(policy);
    auto result_handler = [policy](bool allow) {
      ewk_certificate_policy_decision_allowed_set(policy, allow);
    };

    auto ptr = ewk_certificate_policy_decision_url_get(policy);
    std::string url(ptr ? ptr : "");
    ptr = ewk_certificate_policy_decision_certificate_pem_get(policy);
    std::string pem(ptr ? ptr : "");
    self->listener_->OnCertificateAllowRequest(self->view_,
                                               url,
                                               pem,
                                               result_handler);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "request,certificate,confirm",
                                 certi_callback,
                                 this);
  smart_callbacks_["request,certificate,confirm"] = certi_callback;
}

void WebViewImpl::InitPopupWaitCallback() {
  evas_object_smart_callback_add(ewk_view_,
      "popup,reply,wait,start",
      [](void* user_data, Evas_Object* /*obj*/, void*) {
        WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
        self->internal_popup_opened_ = true;
#ifdef MANUAL_ROTATE_FEATURE_SUPPORT
        self->window_->EnableManualRotation(false);
#endif  // MANUAL_ROTATE_FEATURE_SUPPORT
      }, this);
  evas_object_smart_callback_add(ewk_view_,
      "popup,reply,wait,finish",
      [](void* user_data, Evas_Object* /*obj*/, void*) {
        WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
        self->internal_popup_opened_ = false;
#ifdef MANUAL_ROTATE_FEATURE_SUPPORT
        self->window_->EnableManualRotation(true);
#endif  // MANUAL_ROTATE_FEATURE_SUPPORT
      }, this);
}

void WebViewImpl::InitUsermediaCallback() {
  auto callback = [](Evas_Object*,
                     Ewk_User_Media_Permission_Request* request,
                     void* user_data) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    if (self == NULL || self->listener_ == NULL) {
      ewk_user_media_permission_reply(request, EINA_FALSE);
      return EINA_TRUE;
    }

    ewk_user_media_permission_request_suspend(request);
    const Ewk_Security_Origin* origin =
        ewk_user_media_permission_request_origin_get(request);
    std::stringstream url;
    url << ewk_security_origin_protocol_get(origin)
        << "://"
        << ewk_security_origin_host_get(origin)
        << ":"
        << ewk_security_origin_port_get(origin);

    auto result_handler = [request](bool result) {
      LOGGER(DEBUG) << "Getusermedia Permission Result : " << result;
      ewk_user_media_permission_reply(request, result);
    };
    self->listener_->OnUsermediaPermissionRequest(self->view_,
                                                  url.str(),
                                                  result_handler);
    return EINA_TRUE;
  };
  ewk_view_user_media_permission_callback_set(ewk_view_, callback, this);
}

void WebViewImpl::InitEditorClientImeCallback() {
  auto ime_changed_callback = [](void* user_data,
                           Evas_Object*,
                           void* event_info) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);

    Eina_Rectangle *rect = static_cast<Eina_Rectangle *>(event_info);
    self->ime_width_ = rect->w;
    self->ime_height_ = rect->h;
  };

  auto ime_opened_callback = [](void* user_data,
                           Evas_Object*,
                           void* event_info) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);

    SoftKeyboardChangeEventValue softkeyboard_value;
    softkeyboard_value.state = "on";
    softkeyboard_value.width = self->ime_width_;
    softkeyboard_value.height = self->ime_height_;

    self->listener_->OnSoftKeyboardChangeEvent(self->view_, softkeyboard_value);
  };

  auto ime_closed_callback = [](void* user_data,
                           Evas_Object*,
                           void* event_info) {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);

    SoftKeyboardChangeEventValue softkeyboard_value;
    softkeyboard_value.state = "off";

    self->listener_->OnSoftKeyboardChangeEvent(self->view_, softkeyboard_value);
  };
  evas_object_smart_callback_add(ewk_view_,
                                 "inputmethod,changed",
                                 ime_changed_callback,
                                 this);
  evas_object_smart_callback_add(ewk_view_,
                                 "editorclient,ime,opened",
                                 ime_opened_callback,
                                 this);
  evas_object_smart_callback_add(ewk_view_,
                                 "editorclient,ime,closed",
                                 ime_closed_callback,
                                 this);
  smart_callbacks_["inputmethod,changed"] = ime_changed_callback;
  smart_callbacks_["editorclient,ime,opened"] = ime_opened_callback;
  smart_callbacks_["editorclient,ime,closed"] = ime_closed_callback;
}

#ifdef ROTARY_EVENT_FEATURE_SUPPORT
void WebViewImpl::InitRotaryEventCallback() {
  auto rotary_callback = [](void* user_data,
                         Evas_Object*,
                         Eext_Rotary_Event_Info* event_info) -> Eina_Bool {
    WebViewImpl* self = static_cast<WebViewImpl*>(user_data);
    Eext_Rotary_Event_Info* rotary = event_info;

    RotaryEventType type;
    if (rotary->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE)
      type = RotaryEventType::CLOCKWISE;
    else
      type = RotaryEventType::COUNTER_CLOCKWISE;

    self->listener_->OnRotaryEvent(self->view_, type);
    return EINA_TRUE;
  };

  // add callback to handle rotary event
  eext_rotary_object_event_callback_add(ewk_view_, rotary_callback, this);
  eext_rotary_object_event_activated_set(ewk_view_, EINA_TRUE);
}
#endif  // ROTARY_EVENT_FEATURE_SUPPORT

std::string WebViewImpl::GetUrl() {
  return std::string(ewk_view_url_get(ewk_view_));
}

Evas_Object* WebViewImpl::evas_object() const {
  return ewk_view_;
}

void WebViewImpl::OnRotation(int degree) {
  ewk_view_orientation_send(ewk_view_, ToWebRotation(degree));
}

void WebViewImpl::OnKeyEvent(Eext_Callback_Type key_type) {
  std::string keyname;
  if (key_type == EEXT_CALLBACK_BACK) {
    if (fullscreen_) {
      ewk_view_fullscreen_exit(ewk_view_);
      return;
    }
    if (EINA_TRUE == ewk_view_text_selection_clear(ewk_view_)) {
      return;
    }
    keyname = kKeyNameBack;
  } else if (key_type == EEXT_CALLBACK_MORE) {
    keyname = kKeyNameMenu;
  } else {
    return;
  }

  if (listener_) {
    listener_->OnHardwareKey(view_, keyname);
  }
}

void WebViewImpl::SetEventListener(WebView::EventListener* listener) {
  listener_ = listener;
}

void WebViewImpl::SetAppInfo(const std::string& app_name,
                             const std::string& version) {
  std::string ua = app_name + "/" + version;
  ewk_view_application_name_for_user_agent_set(ewk_view_, ua.c_str());
}
bool WebViewImpl::SetUserAgent(const std::string& user_agent) {
  return ewk_view_user_agent_set(ewk_view_, user_agent.c_str());
}

void WebViewImpl::SetCSPRule(const std::string& rule, bool report_only) {
  ewk_view_content_security_policy_set(
      ewk_view_,
      rule.c_str(),
      report_only ? EWK_REPORT_ONLY : EWK_ENFORCE_POLICY);
}

void WebViewImpl::SetDefaultEncoding(const std::string& encoding) {
  if (ewk_settings_is_encoding_valid(encoding.c_str())) {
    Ewk_Settings* settings = ewk_view_settings_get(ewk_view_);
    ewk_settings_default_text_encoding_name_set(settings, encoding.c_str());
  }
}

#ifdef PROFILE_WEARABLE
void WebViewImpl::SetBGColor(int r, int g, int b, int a) {
  ewk_view_bg_color_set(ewk_view_, r, g, b, a);
}
#endif

}  // namespace runtime
