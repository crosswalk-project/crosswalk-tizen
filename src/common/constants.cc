// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/constants.h"

namespace wrt {

// Extension
const char kSwitchExtensionServer[] = "extension-server";

// DBus for Application
const char kDBusNameForApplication[] = "Application";
const char kDBusInterfaceNameForApplication[] = "org.tizen.wrt.Application";
const char kMethodNotifyEPCreated[] = "NotifyEPCreated";
const char kMethodGetRuntimeVariable[] = "GetRuntimeVariable";

// DBus for Extension
const char kDBusNameForExtension[] = "Extension";
const char kDBusInterfaceNameForExtension[] = "org.tizen.wrt.Extension";
const char kMethodGetExtensions[] = "GetExtensions";
const char kMethodCreateInstance[] = "CreateInstance";
const char kMethodDestroyInstance[] = "DestroyInstance";
const char kMethodSendSyncMessage[] = "SendSyncMessage";
const char kMethodPostMessage[] = "PostMessage";
const char kSignalOnMessageToJS[] = "OnMessageToJS";




}  // namespace wrt
