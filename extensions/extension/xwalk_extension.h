// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_XWALK_EXTENSION_H_
#define XWALK_EXTENSIONS_XWALK_EXTENSION_H_

#include <string>
#include <vector>

#include "extensions/extension/xwalk_extension_instance.h"
#include "extensions/public/XW_Extension.h"
#include "extensions/public/XW_Extension_SyncMessage.h"

namespace extensions {

class XWalkExtensionAdapter;
class XWalkExtensionInstance;

class XWalkExtension {
 public:
  typedef std::vector<std::string> StringVector;

  class XWalkExtensionDelegate {
   public:
    virtual void GetRuntimeVariable(const char* key, char* value,
        size_t value_len) = 0;
  };

  XWalkExtension(const std::string& path, XWalkExtensionDelegate* delegate);
  XWalkExtension(const std::string& path,
                 const std::string& name,
                 const StringVector& entry_points,
                 XWalkExtensionDelegate* delegate);
  virtual ~XWalkExtension();

  bool Initialize();
  XWalkExtensionInstance* CreateInstance();

  std::string name() const { return name_; }

  std::string javascript_api() const { return javascript_api_; }

  const StringVector& entry_points() const {
    return entry_points_;
  }

  bool use_trampoline() const {
    return use_trampoline_;
  }

  bool lazy_loading() const {
    return lazy_loading_;
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
  friend class XWalkExtensionAdapter;
  friend class XWalkExtensionInstance;

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
  bool lazy_loading_;

  XWalkExtensionDelegate* delegate_;

  XW_CreatedInstanceCallback created_instance_callback_;
  XW_DestroyedInstanceCallback destroyed_instance_callback_;
  XW_ShutdownCallback shutdown_callback_;
  XW_HandleMessageCallback handle_msg_callback_;
  XW_HandleSyncMessageCallback handle_sync_msg_callback_;
};

}  // namespace extensions

#endif  // XWALK_EXTENSIONS_XWALK_EXTENSION_H_
