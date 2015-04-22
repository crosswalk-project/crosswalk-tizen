// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_DBUS_CLIENT_H_
#define WRT_COMMON_DBUS_CLIENT_H_

#include <glib.h>
#include <gio/gio.h>

#include <string>

namespace wrt {

class DBusClient {
 public:
  DBusClient();
  virtual ~DBusClient();

  bool Connect(const std::string& address);
  bool ConnectByName(const std::string& name);

  GVariant* Call(const std::string& iface, const std::string& method,
                 GVariant* parameters, const GVariantType* reply_type);
 private:
  GDBusConnection* connection_;
};

}  // namespace wrt

#endif  // WRT_COMMON_DBUS_CLIENT_H_
