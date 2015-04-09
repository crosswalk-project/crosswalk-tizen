// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_MESSAGE_TYPES_H_
#define WRT_COMMON_MESSAGE_TYPES_H_

namespace wrt {

namespace message_types {

extern const char* kExtensionTypePrefix;
extern const char* kExtensionEPCreated;
extern const char* kExtensionGetExtensions;
extern const char* kExtensionCreateInstance;
extern const char* kExtensionDestroyInstance;
extern const char* kExtensionCallSync;
extern const char* kExtensionCallAsync;
extern const char* kExtensionPostMessageToJS;

}  // namespace message_types

}  // namespace wrt

#endif  // WRT_COMMON_MESSAGE_TYPES_H_
