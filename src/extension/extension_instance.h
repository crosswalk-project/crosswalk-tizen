// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_EXTENSION_EXTENSION_INSTANCE_H_
#define WRT_EXTENSION_EXTENSION_INSTANCE_H_

#include <string>
#include <functional>

#include "extension/xwalk/XW_Extension.h"

namespace wrt {

class Extension;

class ExtensionInstance {
 public:
  typedef std::function<void(const std::string&)> MessageCallback;

  ExtensionInstance(Extension* extension, XW_Instance xw_instance);
  virtual ~ExtensionInstance();

  void HandleMessage(const std::string& msg);
  void HandleSyncMessage(const std::string& msg);

  void SetPostMessageCallback(MessageCallback callback);
  void SetSendSyncReplyCallback(MessageCallback callback);

 private:
  friend class ExtensionAdapter;

  void PostMessageToJS(const std::string& msg);
  void SyncReplyToJS(const std::string& reply);

  Extension* extension_;
  XW_Instance xw_instance_;
  void* instance_data_;

  MessageCallback post_message_callback_;
  MessageCallback send_sync_reply_callback_;
};

}  // namespace wrt

#endif  // WRT_EXTENSION_EXTENSION_INSTANCE_H_
