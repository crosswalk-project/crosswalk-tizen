// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_TESTS_POPUP_STRING_H_
#define WRT_TESTS_POPUP_STRING_H_

#include <string>

namespace wrt {

namespace popup_string {

extern const char kTextDomainWrt[];

extern const char kPopupTitleAuthRequest[];
extern const char kPopupTitleCert[];
extern const char kPopupTitleGeoLocation[];
extern const char kPopupTitleUserMedia[];
extern const char kPopupTitleWebNotification[];
extern const char kPopupTitleWebStorage[];

extern const char kPopupBodyAuthRequest[];
extern const char kPopupBodyCert[];
extern const char kPopupBodyGeoLocation[];
extern const char kPopupBodyUserMedia[];
extern const char kPopupBodyWebNotification[];
extern const char kPopupBodyWebStorage[];

extern const char kPopupCheckRememberPreference[];

extern const char kPopupLabelAuthusername[];
extern const char kPopupLabelPassword[];

extern const char kPopupButtonOk[];
extern const char kPopupButtonLogin[];
extern const char kPopupButtonCancel[];
extern const char kPopupButtonAllow[];
extern const char kPopupButtonDeny[];

std::string GetText(const std::string& msg_id);

}  // namespace popup_string

}  // namespace wrt

#endif  // WRT_TESTS_POPUP_STRING_H_
