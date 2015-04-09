// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_EXTENSION_EXTENSION_H_
#define WRT_EXTENSION_EXTENSION_H_

#include <string>
#include <vector>

#include "extension/extension_instance.h"
#include "extension/xwalk/XW_Extension.h"
#include "extension/xwalk/XW_Extension_SyncMessage.h"

namespace wrt {

class ExtensionAdapter;
class ExtensionInstance;

class Extension {
 public:
  typedef std::vector<std::string> StringVector;

  class ExtensionDelegate {
   public:
    virtual void GetRuntimeVariable(const char* key, char* value,
        size_t value_len) = 0;
  };

  Extension(const std::string& path, ExtensionDelegate* delegate);
  virtual ~Extension();

  bool Initialize();
  ExtensionInstance* CreateInstance();

  std::string name() const { return name_; }

  std::string javascript_api() const { return javascript_api_; }

  const StringVector& entry_points() const {
    return entry_points_;
  }

  bool use_trampoline() const {
    return use_trampoline_;
  }

  void set_name(const std::string& name) {
    name_ = name;
  }

  void set_javascript_api(const std::string& javascript_api) {
    javascript_api_ = javascript_api;
  }

  void set_use_trampoline(bool use_trampoline) {
    use_trampoline_ = use_trampoline;
  }

 private:
  friend class ExtensionAdapter;
  friend class ExtensionInstance;

  void GetRuntimeVariable(const char* key, char* value, size_t value_len);
  int CheckAPIAccessControl(const char* api_name);
  int RegisterPermissions(const char* perm_table);

  bool initialized_;
  std::string library_path_;
  XW_Extension xw_extension_;

  std::string name_;
  std::string javascript_api_;
  StringVector entry_points_;
  bool use_trampoline_;

  ExtensionDelegate* delegate_;

  XW_CreatedInstanceCallback created_instance_callback_;
  XW_DestroyedInstanceCallback destroyed_instance_callback_;
  XW_ShutdownCallback shutdown_callback_;
  XW_HandleMessageCallback handle_msg_callback_;
  XW_HandleSyncMessageCallback handle_sync_msg_callback_;
};

}  // namespace wrt

#endif  // WRT_EXTENSION_EXTENSION_H_
