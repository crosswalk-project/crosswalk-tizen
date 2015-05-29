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


#ifndef WRT_RUNTIME_POPUP_STRING_H_
#define WRT_RUNTIME_POPUP_STRING_H_

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

#endif  // WRT_RUNTIME_POPUP_STRING_H_
