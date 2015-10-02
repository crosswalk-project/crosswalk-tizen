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

#include "common/dbus_client.h"

#include "common/file_utils.h"
#include "common/logger.h"

namespace common {

namespace {

void OnSignalReceived(GDBusConnection* /*connection*/,
                      const gchar* /*sender_name*/,
                      const gchar* /*object_path*/,
                      const gchar* interface_name,
                      const gchar* signal_name,
                      GVariant* parameters,
                      gpointer user_data) {
  DBusClient* self = reinterpret_cast<DBusClient*>(user_data);
  auto callback = self->GetSignalCallback(interface_name);
  if (callback) {
    callback(signal_name, parameters);
  }
}

}  // namespace

DBusClient::DBusClient()
    : connection_(NULL),
      signal_subscription_id_(0) {
}

DBusClient::~DBusClient() {
  if (connection_) {
    g_dbus_connection_signal_unsubscribe(connection_, signal_subscription_id_);
    g_dbus_connection_close_sync(connection_, NULL, NULL);
  }
}

bool DBusClient::ConnectByName(const std::string& name) {
  std::string address("unix:path=");
  address.append(utils::GetUserRuntimeDir());
  address.append("/.");
  address.append(name);
  return Connect(address);
}

bool DBusClient::Connect(const std::string& address) {
  GError *err = NULL;
  connection_ = g_dbus_connection_new_for_address_sync(
      address.c_str(),
      G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
      NULL, NULL, &err);
  if (!connection_) {
    LOGGER(ERROR) << "Failed to connect to bus address " << address
                  << " : " << err->message;
    g_error_free(err);
    return false;
  }

  signal_subscription_id_ = g_dbus_connection_signal_subscribe(
      connection_, NULL, NULL, NULL, NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE,
      OnSignalReceived, this, NULL);

  return true;
}

GVariant* DBusClient::Call(const std::string& iface,
                           const std::string& method,
                           GVariant* parameters,
                           const GVariantType* reply_type) {
  if (!connection_) {
    return NULL;
  }

  GError *err = NULL;
  GVariant* reply = NULL;

  if (reply_type) {
    reply = g_dbus_connection_call_sync(
        connection_, NULL, "/", iface.c_str(), method.c_str(), parameters,
        reply_type, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &err);
    if (!reply) {
      LOGGER(ERROR) << "Failed to CallSync : " << err->message;
      g_error_free(err);
    }
  } else {
    g_dbus_connection_call(
        connection_, NULL, "/", iface.c_str(), method.c_str(), parameters,
        NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);
  }

  return reply;
}

void DBusClient::SetSignalCallback(const std::string& iface,
                                   SignalCallback func) {
  signal_callbacks_[iface] = func;
}

DBusClient::SignalCallback
DBusClient::GetSignalCallback(const std::string& iface) {
  return signal_callbacks_[iface];
}

}  // namespace common
