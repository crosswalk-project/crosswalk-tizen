// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/message_types.h"

namespace wrt {

namespace message_types {

const char* kExtensionTypePrefix = "tizen://extension/";
const char* kExtensionEPCreated = "tizen://extension/ep_created";
const char* kExtensionGetExtensions = "tizen://extension/get_extensions";
const char* kExtensionCreateInstance = "tizen://extension/create_instance";
const char* kExtensionDestroyInstance = "tizen://extension/destroy_instance";
const char* kExtensionCallSync = "tizen://extension/call_sync";
const char* kExtensionCallAsync = "tizen://extension/call_async";
const char* kExtensionPostMessageToJS = "tizen://extension/post_message_to_js";

}  // namespace message_types

}  // namespace wrt
