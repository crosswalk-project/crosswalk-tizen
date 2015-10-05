// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/extension/xwalk_extension.h"

#include <dlfcn.h>
#include <string>

#include "common/logger.h"
#include "extensions/extension/xwalk_extension_adapter.h"
#include "extensions/public/XW_Extension.h"

namespace extensions {

XWalkExtension::XWalkExtension(const std::string& path,
                               XWalkExtensionDelegate* delegate)
  : initialized_(false),
    library_path_(path),
    xw_extension_(0),
    use_trampoline_(true),
    lazy_loading_(false),
    delegate_(delegate),
    created_instance_callback_(NULL),
    destroyed_instance_callback_(NULL),
    shutdown_callback_(NULL),
    handle_msg_callback_(NULL),
    handle_sync_msg_callback_(NULL) {
}

XWalkExtension::XWalkExtension(const std::string& path,
                               const std::string& name,
                               const StringVector& entry_points,
                               XWalkExtensionDelegate* delegate)
  : initialized_(false),
    library_path_(path),
    xw_extension_(0),
    name_(name),
    entry_points_(entry_points),
    use_trampoline_(true),
    lazy_loading_(true),
    delegate_(delegate),
    created_instance_callback_(NULL),
    destroyed_instance_callback_(NULL),
    shutdown_callback_(NULL),
    handle_msg_callback_(NULL),
    handle_sync_msg_callback_(NULL) {
}

XWalkExtension::~XWalkExtension() {
  if (!initialized_)
    return;

  if (shutdown_callback_)
    shutdown_callback_(xw_extension_);
  XWalkExtensionAdapter::GetInstance()->UnregisterExtension(this);
}

bool XWalkExtension::Initialize() {
  if (initialized_)
    return true;

  void* handle = dlopen(library_path_.c_str(), RTLD_LAZY);
  if (!handle) {
    LOGGER(ERROR) << "Error loading extension '"
                  << library_path_ << "' : " << dlerror();
    return false;
  }

  XW_Initialize_Func initialize = reinterpret_cast<XW_Initialize_Func>(
      dlsym(handle, "XW_Initialize"));
  if (!initialize) {
    LOGGER(ERROR) << "Error loading extension '" << library_path_
                  << "' : couldn't get XW_Initialize function.";
    dlclose(handle);
    return false;
  }

  XWalkExtensionAdapter* adapter = XWalkExtensionAdapter::GetInstance();
  xw_extension_ = adapter->GetNextXWExtension();
  adapter->RegisterExtension(this);

  int ret = initialize(xw_extension_, XWalkExtensionAdapter::GetInterface);
  if (ret != XW_OK) {
    LOGGER(ERROR) << "Error loading extension '" << library_path_
                  << "' : XW_Initialize() returned error value.";
    dlclose(handle);
    return false;
  }

  initialized_ = true;
  return true;
}

XWalkExtensionInstance* XWalkExtension::CreateInstance() {
  Initialize();
  XWalkExtensionAdapter* adapter = XWalkExtensionAdapter::GetInstance();
  XW_Instance xw_instance = adapter->GetNextXWInstance();
  return new XWalkExtensionInstance(this, xw_instance);
}

void XWalkExtension::GetRuntimeVariable(const char* key, char* value,
    size_t value_len) {
  if (delegate_) {
    delegate_->GetRuntimeVariable(key, value, value_len);
  }
}
int XWalkExtension::CheckAPIAccessControl(const char* /*api_name*/) {
  // Not Supported
  return XW_OK;
}

int XWalkExtension::RegisterPermissions(const char* /*perm_table*/) {
  // Not Supported
  return XW_OK;
}

}  // namespace extensions

