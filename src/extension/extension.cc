// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extension/extension.h"

#include <dlfcn.h>
#include <string>

#include "common/logger.h"
#include "extension/extension_adapter.h"
#include "extension/xwalk/XW_Extension.h"

namespace wrt {

Extension::Extension(const std::string& path, ExtensionDelegate* delegate)
  : initialized_(false),
    library_path_(path),
    xw_extension_(0),
    use_trampoline_(true),
    delegate_(delegate),
    created_instance_callback_(NULL),
    destroyed_instance_callback_(NULL),
    shutdown_callback_(NULL),
    handle_msg_callback_(NULL),
    handle_sync_msg_callback_(NULL) {
}

Extension::~Extension() {
  if (!initialized_)
    return;

  if (shutdown_callback_)
    shutdown_callback_(xw_extension_);
  ExtensionAdapter::GetInstance()->UnregisterExtension(this);
}

bool Extension::Initialize() {
  if (initialized_)
    return true;

  void* handle = dlopen(library_path_.c_str(), RTLD_LAZY);
  if (!handle) {
    LoggerE("Error loading extension '%s' : %s",
            library_path_.c_str(), dlerror());
    return false;
  }

  XW_Initialize_Func initialize = reinterpret_cast<XW_Initialize_Func>(
      dlsym(handle, "XW_Initialize"));
  if (!initialize) {
    LoggerE(
        "Error loading extension '%s' : couldn't get XW_Initialize function",
        library_path_.c_str());
    dlclose(handle);
    return false;
  }

  ExtensionAdapter* adapter = ExtensionAdapter::GetInstance();
  xw_extension_ = adapter->GetNextXWExtension();
  adapter->RegisterExtension(this);

  int ret = initialize(xw_extension_, ExtensionAdapter::GetInterface);
  if (ret != XW_OK) {
    LoggerE(
        "Error loading extension '%s' : XW_Initialize() returned error value.",
        library_path_.c_str());
    dlclose(handle);
    return false;
  }

  initialized_ = true;
  return true;
}

ExtensionInstance* Extension::CreateInstance() {
  ExtensionAdapter* adapter = ExtensionAdapter::GetInstance();
  XW_Instance xw_instance = adapter->GetNextXWInstance();
  return new ExtensionInstance(this, xw_instance);
}

void Extension::GetRuntimeVariable(const char* key, char* value,
    size_t value_len) {
  if (delegate_) {
    delegate_->GetRuntimeVariable(key, value, value_len);
  }
}
int Extension::CheckAPIAccessControl(const char* /*api_name*/) {
  // Not Supported
  return XW_OK;
}

int Extension::RegisterPermissions(const char* /*perm_table*/) {
  // Not Supported
  return XW_OK;
}

}  // namespace wrt

