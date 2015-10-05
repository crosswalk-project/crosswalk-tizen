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

#ifndef XWALK_COMMON_DBUS_CLIENT_H_
#define XWALK_COMMON_DBUS_CLIENT_H_

#include <gio/gio.h>
#include <glib.h>

#include <functional>
#include <map>
#include <string>

namespace common {

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

}  // namespace common

#endif  // XWALK_COMMON_DBUS_CLIENT_H_
