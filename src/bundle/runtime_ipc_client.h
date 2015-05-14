// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_BUNDLE_RUNTIME_IPC_CLIENT_H_
#define WRT_BUNDLE_RUNTIME_IPC_CLIENT_H_

#include <v8/v8.h>
#include <ewk_ipc_message.h>
#include <functional>
#include <map>
#include <string>

namespace wrt {

class RuntimeIPCClient {
 public:
  class JSCallback {
   public:
    explicit JSCallback(v8::Isolate* isolate,
                        v8::Handle<v8::Function> callback);
    ~JSCallback();

    v8::Isolate* isolate() const { return isolate_; }

    void Call(v8::Handle<v8::Value> args[]);
   private:
    v8::Isolate* isolate_;
    v8::Persistent<v8::Function> callback_;
  };

  typedef std::function<void(const std::string& type,
                             const std::string& value,
                             JSCallback* js_callback)> ReplyCallback;

  static RuntimeIPCClient* GetInstance();

  // Send message to BrowserProcess without reply
  void SendMessage(const std::string& type, const std::string& value);

  // Send message to BrowserProcess synchronous with reply
  std::string SendSyncMessage(const std::string& type, const std::string& value);

  // Send message to BrowserProcess asynchronous,
  // reply message will be passed to callback function.
  void SendAsyncMessage(const std::string& type, const std::string& value,
                        ReplyCallback callback, JSCallback* js_callback);

  void HandleMessageFromRuntime(const Ewk_IPC_Wrt_Message_Data* msg);

  int routing_id() const { return routing_id_; }
  void set_routing_id(int routing_id) { routing_id_ = routing_id; }

 private:
  class AsyncData {
   public:
    ~AsyncData() {
      if (js_callback) delete js_callback;
    }

    ReplyCallback callback;
    JSCallback* js_callback;
  };

  RuntimeIPCClient();

  int routing_id_;
  std::map<std::string, AsyncData> callbacks_;
};

}  // namespace

#endif  // WRT_BUNDLE_RUNTIME_IPC_CLIENT_H_