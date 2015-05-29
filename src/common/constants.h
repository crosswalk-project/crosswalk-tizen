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

#ifndef WRT_COMMON_CONSTANTS_H_
#define WRT_COMMON_CONSTANTS_H_

namespace wrt {

extern const char kSwitchExtensionServer[];

extern const char kDBusNameForApplication[];
extern const char kDBusInterfaceNameForApplication[];
extern const char kMethodNotifyEPCreated[];
extern const char kMethodGetRuntimeVariable[];

extern const char kDBusNameForExtension[];
extern const char kDBusInterfaceNameForExtension[];
extern const char kMethodGetExtensions[];
extern const char kMethodCreateInstance[];
extern const char kMethodDestroyInstance[];
extern const char kMethodSendSyncMessage[];
extern const char kMethodPostMessage[];
extern const char kSignalOnMessageToJS[];

}  // namespace wrt

#endif  // WRT_COMMON_CONSTANTS_H_
