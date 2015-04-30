// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_POPUP_STRING_H_
#define WRT_RUNTIME_POPUP_STRING_H_

#include <string>

namespace wrt {

namespace popup_string {

static const char* kTextDomainWrt = "wrt";

static const char* kPopupTitleAuthRequest = "Auth Request";
static const char* kPopupTitleCert = "IDS_BR_HEADER_CERTIFICATE_INFO";
static const char* kPopupTitleGeoLocation = "IDS_WRT_OPT_ACCESS_USER_LOCATION";
static const char* kPopupTitleUserMedia = "IDS_WRT_OPT_USE_USER_MEDIA";
static const char* kPopupTitleWebNotification =
  "IDS_BR_HEADER_WEB_NOTIFICATION";
static const char* kPopupTitleWebStorage = "IDS_WRT_OPT_USE_STORE_WEB_DATA";

static const char* kPopupBodyAuthRequest =
  "IDS_BR_BODY_DESTINATIONS_AUTHENTICATION_REQUIRED";
static const char* kPopupBodyCert =
  "IDS_BR_BODY_SECURITY_CERTIFICATE_PROBLEM_MSG";
static const char* kPopupBodyGeoLocation =
  "IDS_WRT_BODY_ALLOWS_THIS_SITE_TO_ACCESS_YOUR_LOCATION_INFORMATION";
static const char* kPopupBodyUserMedia =
  "IDS_WRT_BODY_ALLOWS_THIS_SITE_TO_USE_THE_MEDIA_FILES_STORED_ON_YOUR_DEVICE";
static const char* kPopupBodyWebNotification =
  "IDS_WRT_BODY_ALLOWS_THIS_SITE_TO_DISPLAY_NOTIFICATIONS";
static const char* kPopupBodyWebStorage =
  "IDS_WRT_BODY_ALLOWS_THIS_SITE_TO_SAVE_A_LARGE_AMOUNT_OF_DATA_ON_YOUR_DEVICE";

static const char* kPopupCheckRememberPreference =
  "IDS_BR_BODY_REMEMBER_PREFERENCE";

static const char* kPopupLabelAuthusername = "IDS_BR_BODY_AUTHUSERNAME";
static const char* kPopupLabelPassword =  "IDS_BR_BODY_AUTHPASSWORD";

static const char* kPopupButtonOk = "IDS_BR_SK_OK";
static const char* kPopupButtonLogin = "IDS_BR_BODY_LOGIN";
static const char* kPopupButtonCancel = "IDS_BR_SK_CANCEL";
static const char* kPopupButtonAllow = "IDS_BR_OPT_ALLOW";
static const char* kPopupButtonDeny = "IDS_COM_BODY_DENY";

static std::string GetText(const std::string& msg_id) {
  return dgettext(kTextDomainWrt, msg_id.c_str());
}

}  // namespace popup_string

}  // namespace wrt

#endif  // WRT_RUNTIME_POPUP_STRING_H_
