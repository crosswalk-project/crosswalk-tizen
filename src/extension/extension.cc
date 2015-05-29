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

  ExtensionAdapter* adapter = ExtensionAdapter::GetInstance();
  xw_extension_ = adapter->GetNextXWExtension();
  adapter->RegisterExtension(this);

  int ret = initialize(xw_extension_, ExtensionAdapter::GetInterface);
  if (ret != XW_OK) {
    LOGGER(ERROR) << "Error loading extension '" << library_path_
                  << "' : XW_Initialize() returned error value.";
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

