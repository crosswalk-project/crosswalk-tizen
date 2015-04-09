// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extension/extension_server.h"

#include <Elementary.h>
#include <glob.h>
#include <string>
#include <vector>

#include "common/logger.h"
#include "common/file_utils.h"
#include "common/message_types.h"
#include "common/picojson.h"
#include "extension/extension.h"

namespace wrt {

namespace {
const char kExtensionDir[] = "/usr/lib/tizen-extensions-crosswalk";
const char kExtensionPrefix[] = "lib";
const char kExtensionSuffix[] = ".so";
}  // namespace

ExtensionServer::ExtensionServer(Ewk_Context* ewk_context)
    : ewk_context_(ewk_context) {
}

ExtensionServer::~ExtensionServer() {
}

void ExtensionServer::RegisterExtension(const std::string& path) {
  Extension* ext = new Extension(path, this);
  if (!ext->Initialize() || !RegisterSymbols(ext)) {
    delete ext;
  }
  extensions_[ext->name()] = ext;
}

void ExtensionServer::RegisterSystemExtensions() {
  std::string extension_path(kExtensionDir);
  extension_path.append("/");
  extension_path.append(kExtensionPrefix);
  extension_path.append("*");
  extension_path.append(kExtensionSuffix);

  glob_t glob_result;
  glob(extension_path.c_str(), GLOB_TILDE, NULL, &glob_result);
  for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
    RegisterExtension(glob_result.gl_pathv[i]);
  }
}

bool ExtensionServer::RegisterSymbols(Extension* extension) {
  std::string name = extension->name();

  if (extension_symbols_.find(name) != extension_symbols_.end()) {
    LoggerW("Ignoring extension with name already registred. '%s'",
            name.c_str());
    return false;
  }

  Extension::StringVector entry_points = extension->entry_points();
  for (auto it = entry_points.begin(); it != entry_points.end(); ++it) {
    if (extension_symbols_.find(*it) != extension_symbols_.end()) {
      LoggerW("Ignoring extension with entry_point already registred. '%s'",
              (*it).c_str());
      return false;
    }
  }

  for (auto it = entry_points.begin(); it != entry_points.end(); ++it) {
    extension_symbols_.insert(*it);
  }

  extension_symbols_.insert(name);

  return true;
}

void ExtensionServer::AddRuntimeVariable(const std::string& key,
    const std::string& value) {
  runtime_variables_.insert(std::make_pair(key, value));
}

void ExtensionServer::ClearRuntimeVariables() {
  runtime_variables_.clear();
}

void ExtensionServer::GetRuntimeVariable(const char* key, char* value,
    size_t value_len) {
  auto it = runtime_variables_.find(key);
  if (it != runtime_variables_.end()) {
    strncpy(value, it->second.c_str(), value_len);
  }
}

void ExtensionServer::Start() {
  Start(StringVector());
}

void ExtensionServer::Start(const StringVector& paths) {
  // Register system extensions to support Tizen Device APIs
  RegisterSystemExtensions();

  // Register user extensions
  for (auto it = paths.begin(); it != paths.end(); ++it) {
    if (utils::Exists(*it)) {
      RegisterExtension(*it);
    }
  }

  // Send 'ready' signal to Injected Bundle.
  SendWrtMessage(message_types::kExtensionEPCreated);
}

void ExtensionServer::HandleWrtMessage(Ewk_IPC_Wrt_Message_Data* message) {
  if (!message) {
    LoggerW("Message object is NULL.");
    return;
  }

  if (!ewk_context_) {
    LoggerW("Ewk_Context is NULL.");
    return;
  }

  Eina_Stringshare* msg_id = ewk_ipc_wrt_message_data_id_get(message);
  Eina_Stringshare* msg_reference_id
                        = ewk_ipc_wrt_message_data_reference_id_get(message);
  Eina_Stringshare* msg_type = ewk_ipc_wrt_message_data_type_get(message);
  Eina_Stringshare* msg_value = ewk_ipc_wrt_message_data_value_get(message);

  Ewk_IPC_Wrt_Message_Data* reply = ewk_ipc_wrt_message_data_new();

  #define START_WITHS(x, s) (strncmp(x, s, strlen(s)) == 0)
  if (START_WITHS(msg_type, message_types::kExtensionGetExtensions)) {
    OnGetExtensions(msg_id);
  } else if (START_WITHS(msg_type, message_types::kExtensionCreateInstance)) {
    OnCreateInstance(msg_reference_id, msg_value);
  } else if (START_WITHS(msg_type, message_types::kExtensionDestroyInstance)) {
    OnDestroyInstance(msg_reference_id);
  } else if (START_WITHS(msg_type, message_types::kExtensionCallSync)) {
    OnSendSyncMessageToNative(msg_id, msg_reference_id, msg_value);
  } else if (START_WITHS(msg_type, message_types::kExtensionCallAsync)) {
    OnPostMessageToNative(msg_reference_id, msg_value);
  }

  eina_stringshare_del(msg_id);
  eina_stringshare_del(msg_reference_id);
  eina_stringshare_del(msg_type);
  eina_stringshare_del(msg_value);
  ewk_ipc_wrt_message_data_del(reply);

  #undef START_WITHS
}

void ExtensionServer::SendWrtMessage(const std::string& type) {
  SendWrtMessage(type, std::string(), std::string(), std::string());
}

void ExtensionServer::SendWrtMessage(
    const std::string& type, const std::string& id,
    const std::string& ref_id, const std::string& value) {
  if (!ewk_context_) {
    LoggerW("Ewk_Context is NULL.");
    return;
  }
  Ewk_IPC_Wrt_Message_Data* msg = ewk_ipc_wrt_message_data_new();
  ewk_ipc_wrt_message_data_type_set(msg, type.c_str());
  ewk_ipc_wrt_message_data_id_set(msg, id.c_str());
  ewk_ipc_wrt_message_data_reference_id_set(msg, ref_id.c_str());
  ewk_ipc_wrt_message_data_value_set(msg, value.c_str());
  if (!ewk_ipc_wrt_message_send(ewk_context_, msg)) {
    LoggerE("Failed to send message to injected bundle.");
  }
  ewk_ipc_wrt_message_data_del(msg);
}

void ExtensionServer::OnGetExtensions(const std::string& id) {
  picojson::array ext_array = picojson::array(0);
  for (auto it = extensions_.begin(); it != extensions_.end(); ++it) {
    picojson::object ext_obj = picojson::object();
    ext_obj["name"] = picojson::value(it->second->name());
    ext_obj["javascript_api"] = picojson::value(it->second->javascript_api());
    ext_obj["entry_points"] = picojson::value(picojson::array(0));
    picojson::array ext_ep_array
        = ext_obj["entry_points"].get<picojson::array>();
    Extension::StringVector entry_points = it->second->entry_points();
    for (auto it_ep = entry_points.begin();
         it_ep != entry_points.end(); ++it_ep) {
      ext_ep_array.push_back(picojson::value(*it_ep));
    }
    ext_array.push_back(picojson::value(ext_obj));
  }

  picojson::value reply(ext_array);
  SendWrtMessage(message_types::kExtensionGetExtensions,
                 std::string(), id, reply.serialize());
}

void ExtensionServer::OnCreateInstance(
    const std::string& instance_id, const std::string& extension_name) {
  // find extension with given name
  auto it = extensions_.find(extension_name);
  if (it == extensions_.end()) {
    LoggerE("Can't find extension [%s].", extension_name.c_str());
    return;
  }

  // create instance
  ExtensionInstance* instance = it->second->CreateInstance();
  if (!instance) {
    LoggerE("Can't create instance of extension [%s].", extension_name.c_str());
    return;
  }

  // Set PostMessageCallback / SendSyncReplyCallback
  using std::placeholders::_1;
  instance->SetPostMessageCallback(
    std::bind(&ExtensionServer::OnPostMessageToJS, this, instance_id, _1));
  instance->SetSendSyncReplyCallback(
    std::bind(&ExtensionServer::OnSendSyncReplyToJS, this, instance_id, _1));

  instances_[instance_id] = instance;
}

void ExtensionServer::OnDestroyInstance(const std::string& instance_id) {
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LoggerE("Can't find instance [%s].", instance_id.c_str());
    return;
  }

  ExtensionInstance* instance = it->second;
  delete instance;

  instances_.erase(it);
}

void ExtensionServer::OnSendSyncMessageToNative(
    const std::string& msg_id, const std::string& instance_id,
    const std::string& msg_body) {
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LoggerE("Can't find instance [%s].", instance_id.c_str());
    SendWrtMessage(message_types::kExtensionCallSync, "", msg_id, "ERROR");
    return;
  }

  ExtensionInstance* instance = it->second;
  instance->HandleSyncMessage(msg_body);
}

void ExtensionServer::OnPostMessageToNative(
    const std::string& instance_id, const std::string& msg_body) {
  auto it = instances_.find(instance_id);
  if (it == instances_.end()) {
    LoggerE("Can't find instance [%s].", instance_id.c_str());
    return;
  }

  ExtensionInstance* instance = it->second;
  instance->HandleMessage(msg_body);
}

void ExtensionServer::OnPostMessageToJS(
    const std::string& instance_id, const std::string& msg) {
  SendWrtMessage(message_types::kExtensionPostMessageToJS,
                 "", instance_id, msg);
}

void ExtensionServer::OnSendSyncReplyToJS(
    const std::string& instance_id, const std::string& msg) {
  SendWrtMessage(message_types::kExtensionCallSync,
                 "", instance_id, msg);
}

}  // namespace wrt
