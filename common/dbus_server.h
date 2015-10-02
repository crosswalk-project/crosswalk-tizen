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

#ifndef XWALK_COMMON_DBUS_SERVER_H_
#define XWALK_COMMON_DBUS_SERVER_H_

#include <gio/gio.h>
#include <glib.h>
#include <functional>
#include <map>
#include <string>

namespace common {

class DBusServer {
 public:
  typedef std::function<bool(GCredentials* creds)> PeerCredentialsCallback;
  typedef std::function<void(GDBusConnection* connection,
                             const std::string& method_name,
                             GVariant* parameters,
                             GDBusMethodInvocation* invocation)> MethodCallback;
  typedef std::function<GVariant*(GDBusConnection* connection,
                                  const gchar* property)> PropertyGetter;
  typedef std::function<bool(GDBusConnection* connection,
                             const gchar* property,
                             GVariant* value)> PropertySetter;
  typedef std::function<void(GDBusConnection* connection)> DisconnectedCallback;

  DBusServer();
  virtual ~DBusServer();

  void Start(const std::string& name);

  std::string GetClientAddress() const;

  void SetIntrospectionXML(const std::string& xml);
  GDBusNodeInfo* GetIntrospectionNodeInfo() const { return node_info_; }

  void SendSignal(GDBusConnection* connection,
                  const std::string& iface, const std::string& signal_name,
                  GVariant* parameters);

  void SetDisconnectedCallback(DisconnectedCallback func);
  void SetPeerCredentialsCallback(PeerCredentialsCallback func);
  void SetMethodCallback(const std::string& iface, MethodCallback func);
  void SetPropertyGetter(const std::string& iface, PropertyGetter func);
  void SetPropertySetter(const std::string& iface, PropertySetter func);
  DisconnectedCallback GetDisconnectedCallback() const;
  PeerCredentialsCallback GetPeerCredentialsCallback() const;
  MethodCallback GetMethodCallback(const std::string& iface);
  PropertySetter GetPropertySetter(const std::string& iface);
  PropertyGetter GetPropertyGetter(const std::string& iface);

 private:
  std::string address_path_;
  GDBusServer* server_;
  GDBusNodeInfo* node_info_;

  DisconnectedCallback disconnected_callback_;
  PeerCredentialsCallback peer_credentials_callback_;
  std::map<std::string, MethodCallback> method_callbacks_;
  std::map<std::string, PropertyGetter> property_getters_;
  std::map<std::string, PropertySetter> property_setters_;
};

}  // namespace common

#endif  // XWALK_COMMON_DBUS_SERVER_H_
