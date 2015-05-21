// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_CONSTANTS_H_
#define WRT_COMMON_CONSTANTS_H_

namespace wrt {

extern const char kSystemExtensionPath[];
extern const char kExtensionPrefix[];
extern const char kExtensionSuffix[];
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
