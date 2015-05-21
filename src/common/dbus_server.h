// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_DBUS_SERVER_H_
#define WRT_COMMON_DBUS_SERVER_H_

#include <glib.h>
#include <gio/gio.h>
#include <string>
#include <map>
#include <functional>

namespace wrt {

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

  DBusServer();
  virtual ~DBusServer();

  void Start(const std::string& name);

  std::string GetClientAddress() const;

  void SetIntrospectionXML(const std::string& xml);
  GDBusNodeInfo* GetIntrospectionNodeInfo() const { return node_info_; }

  void SendSignal(GDBusConnection* connection,
                  const std::string& iface, const std::string& signal_name,
                  GVariant* parameters);

  void SetPeerCredentialsCallback(PeerCredentialsCallback func);
  void SetMethodCallback(const std::string& iface, MethodCallback func);
  void SetPropertyGetter(const std::string& iface, PropertyGetter func);
  void SetPropertySetter(const std::string& iface, PropertySetter func);
  PeerCredentialsCallback GetPeerCredentialsCallback() const;
  MethodCallback GetMethodCallback(const std::string& iface);
  PropertySetter GetPropertySetter(const std::string& iface);
  PropertyGetter GetPropertyGetter(const std::string& iface);

 private:
  std::string address_path_;
  GDBusServer* server_;
  GDBusNodeInfo* node_info_;

  PeerCredentialsCallback peer_credentials_callback_;
  std::map<std::string, MethodCallback> method_callbacks_;
  std::map<std::string, PropertyGetter> property_getters_;
  std::map<std::string, PropertySetter> property_setters_;
};

}  // namespace wrt

#endif  // WRT_COMMON_DBUS_SERVER_H_
