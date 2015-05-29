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
