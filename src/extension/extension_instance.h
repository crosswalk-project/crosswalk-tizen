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
