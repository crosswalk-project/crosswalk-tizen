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

#ifndef XWALK_EXTENSIONS_RENDERER_RUNTIME_IPC_CLIENT_H_
#define XWALK_EXTENSIONS_RENDERER_RUNTIME_IPC_CLIENT_H_

#include <v8/v8.h>
#include <ewk_ipc_message.h>

#include <functional>
#include <map>
#include <string>

namespace extensions {

class RuntimeIPCClient {
 public:
  class JSCallback {
   public:
    explicit JSCallback(v8::Isolate* isolate,
                        v8::Handle<v8::Function> callback);
    ~JSCallback();

    void Call(v8::Isolate* isolate, v8::Handle<v8::Value> args[]);
   private:
    v8::Persistent<v8::Function> callback_;
  };

  typedef std::function<void(const std::string& type,
                             const std::string& value)> ReplyCallback;

  static RuntimeIPCClient* GetInstance();

  // Send message to BrowserProcess without reply
  void SendMessage(const std::string& type, const std::string& value);

  // Send message to BrowserProcess synchronous with reply
  std::string SendSyncMessage(const std::string& type,
                              const std::string& value);

  // Send message to BrowserProcess asynchronous,
  // reply message will be passed to callback function.
  void SendAsyncMessage(const std::string& type, const std::string& value,
                        ReplyCallback callback);

  void HandleMessageFromRuntime(const Ewk_IPC_Wrt_Message_Data* msg);

  int routing_id() const { return routing_id_; }
  void set_routing_id(int routing_id) { routing_id_ = routing_id; }

 private:
  RuntimeIPCClient();

  int routing_id_;
  std::map<std::string, ReplyCallback> callbacks_;
};

}  // namespace extensions

#endif  // XWALK_EXTENSIONS_RENDERER_RUNTIME_IPC_CLIENT_H_
