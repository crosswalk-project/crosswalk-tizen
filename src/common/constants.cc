// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/constants.h"

namespace wrt {

const char kSystemExtensionPath[] = "/usr/lib/tizen-extensions-crosswalk";
const char kExtensionPrefix[] = "lib";
const char kExtensionSuffix[] = ".so";

const char kDBusNameForRuntime[] = "Runtime";
const char kDBusInterfaceNameForRuntime[] = "org.tizen.wrt.Runtime";
const char kMethodNotifyEPCreated[] = "NotifyEPCreated";
const char kMethodGetRuntimeVariable[] = "GetRuntimeVariable";

const char kDBusNameForExtension[] = "Extension";
const char kDBusInterfaceNameForExtension[] = "org.tizen.wrt.Extension";

}  // namespace wrt
