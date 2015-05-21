// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_DBUS_CLIENT_H_
#define WRT_COMMON_DBUS_CLIENT_H_

#include <glib.h>
#include <gio/gio.h>

#include <string>
#include <functional>
#include <map>

namespace wrt {

class DBusClient {
 public:
  typedef std::function<void(const std::string& signal,
                             GVariant* parameters)> SignalCallback;

  DBusClient();
  virtual ~DBusClient();

  bool Connect(const std::string& address);
  bool ConnectByName(const std::string& name);

  GVariant* Call(const std::string& iface, const std::string& method,
                 GVariant* parameters, const GVariantType* reply_type);

  void SetSignalCallback(const std::string& iface, SignalCallback func);
  SignalCallback GetSignalCallback(const std::string& iface);

 private:
  GDBusConnection* connection_;
  guint signal_subscription_id_;
  std::map<std::string, SignalCallback> signal_callbacks_;
};

}  // namespace wrt

#endif  // WRT_COMMON_DBUS_CLIENT_H_
