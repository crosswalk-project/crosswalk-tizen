// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bundle/runtime_ipc_client.h"

#include "common/logger.h"
#include "common/string_utils.h"

namespace wrt {

RuntimeIPCClient::JSCallback::JSCallback(v8::Isolate* isolate,
                                         v8::Handle<v8::Function> callback)
    : isolate_(isolate) {
  callback_.Reset(isolate, callback);
}

RuntimeIPCClient::JSCallback::~JSCallback() {
  callback_.Reset();
}

void RuntimeIPCClient::JSCallback::Call(v8::Handle<v8::Value> args[]) {
  if (!callback_.IsEmpty()) {
    v8::HandleScope handle_scope(isolate_);
    v8::Handle<v8::Function> func =
        v8::Local<v8::Function>::New(isolate_, callback_);
    func->Call(func, 1, args);
  }
}

// static
RuntimeIPCClient* RuntimeIPCClient::GetInstance() {
  static RuntimeIPCClient self;
  return &self;
}

RuntimeIPCClient::RuntimeIPCClient() : routing_id_(0) {
}

void RuntimeIPCClient::SendMessage(const std::string& type,
                                   const std::string& value) {
  Ewk_IPC_Wrt_Message_Data* msg = ewk_ipc_wrt_message_data_new();
  ewk_ipc_wrt_message_data_id_set(msg, "");
  ewk_ipc_wrt_message_data_type_set(msg, type.c_str());
  ewk_ipc_wrt_message_data_value_set(msg, value.c_str());

  if (routing_id_ > 0) {
    if (!ewk_ipc_plugins_message_send(routing_id_, msg)) {
      LOGGER(ERROR) << "Failed to send message to runtime using ewk_ipc.";
    }
  }

  ewk_ipc_wrt_message_data_del(msg);
}

std::string RuntimeIPCClient::SendSyncMessage(const std::string& type,
                                              const std::string& value) {
  Ewk_IPC_Wrt_Message_Data* msg = ewk_ipc_wrt_message_data_new();
  ewk_ipc_wrt_message_data_type_set(msg, type.c_str());
  ewk_ipc_wrt_message_data_value_set(msg, value.c_str());

  if (routing_id_ > 0) {
    if (!ewk_ipc_plugins_sync_message_send(routing_id_, msg)) {
      LOGGER(ERROR) << "Failed to send message to runtime using ewk_ipc.";
      ewk_ipc_wrt_message_data_del(msg);
      return std::string();
    }
  }

  Eina_Stringshare* msg_value = ewk_ipc_wrt_message_data_value_get(msg);

  std::string result(msg_value);
  eina_stringshare_del(msg_value);
  ewk_ipc_wrt_message_data_del(msg);

  return result;
}

void RuntimeIPCClient::SendAsyncMessage(const std::string& type,
                                        const std::string& value,
                                        ReplyCallback callback,
                                        JSCallback* js_callback) {
  std::string msg_id = utils::GenerateUUID();

  Ewk_IPC_Wrt_Message_Data* msg = ewk_ipc_wrt_message_data_new();
  ewk_ipc_wrt_message_data_id_set(msg, msg_id.c_str());
  ewk_ipc_wrt_message_data_type_set(msg, type.c_str());
  ewk_ipc_wrt_message_data_value_set(msg, value.c_str());

  if (routing_id_ > 0) {
    if (!ewk_ipc_plugins_message_send(routing_id_, msg)) {
      LOGGER(ERROR) << "Failed to send message to runtime using ewk_ipc.";
      ewk_ipc_wrt_message_data_del(msg);
      return;
    }
  }

  callbacks_[msg_id].callback = callback;
  callbacks_[msg_id].js_callback = js_callback;

  ewk_ipc_wrt_message_data_del(msg);
}

void RuntimeIPCClient::HandleMessageFromRuntime(
    const Ewk_IPC_Wrt_Message_Data* msg) {
  if (msg == NULL) {
    LOGGER(ERROR) << "received message is NULL";
    return;
  }

  Eina_Stringshare* msg_refid = ewk_ipc_wrt_message_data_reference_id_get(msg);

  if (msg_refid == NULL || !strcmp(msg_refid,"")) {
    if (msg_refid) eina_stringshare_del(msg_refid);
    LOGGER(ERROR) << "No reference id of received message.";
    return;
  }

  auto it = callbacks_.find(msg_refid);
  if (it == callbacks_.end()) {
    eina_stringshare_del(msg_refid);
    LOGGER(ERROR) << "No registered callback with reference id : " << msg_refid;
    return;
  }

  Eina_Stringshare* msg_type = ewk_ipc_wrt_message_data_type_get(msg);
  Eina_Stringshare* msg_value = ewk_ipc_wrt_message_data_value_get(msg);

  const AsyncData& async_data = it->second;
  async_data.callback(msg_type, msg_value, async_data.js_callback);
  callbacks_.erase(it);

  eina_stringshare_del(msg_refid);
  eina_stringshare_del(msg_type);
  eina_stringshare_del(msg_value);
}

}  // namespace wrt
