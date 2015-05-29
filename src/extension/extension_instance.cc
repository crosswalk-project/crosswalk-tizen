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
