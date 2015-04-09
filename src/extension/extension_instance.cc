// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extension/extension_instance.h"

#include "extension/extension_adapter.h"
#include "extension/xwalk/XW_Extension_SyncMessage.h"

namespace wrt {

ExtensionInstance::ExtensionInstance(Extension* extension,
    XW_Instance xw_instance)
  : extension_(extension),
    xw_instance_(xw_instance),
    instance_data_(NULL) {
  ExtensionAdapter::GetInstance()->RegisterInstance(this);
  XW_CreatedInstanceCallback callback = extension_->created_instance_callback_;
  if (callback)
    callback(xw_instance_);
}

ExtensionInstance::~ExtensionInstance() {
  XW_DestroyedInstanceCallback callback =
      extension_->destroyed_instance_callback_;
  if (callback)
    callback(xw_instance_);
  ExtensionAdapter::GetInstance()->UnregisterInstance(this);
}

void ExtensionInstance::HandleMessage(const std::string& msg) {
  XW_HandleMessageCallback callback = extension_->handle_msg_callback_;
  if (callback)
    callback(xw_instance_, msg.c_str());
}

void ExtensionInstance::HandleSyncMessage(const std::string& msg) {
  XW_HandleSyncMessageCallback callback = extension_->handle_sync_msg_callback_;
  if (callback) {
    callback(xw_instance_, msg.c_str());
  }
}

void ExtensionInstance::SetPostMessageCallback(MessageCallback callback) {
  post_message_callback_ = callback;
}

void ExtensionInstance::SetSendSyncReplyCallback(MessageCallback callback) {
  send_sync_reply_callback_ = callback;
}

void ExtensionInstance::PostMessageToJS(const std::string& msg) {
  post_message_callback_(msg);
}

void ExtensionInstance::SyncReplyToJS(const std::string& reply) {
  send_sync_reply_callback_(reply);
}

}  // namespace wrt
