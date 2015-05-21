// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_BUNDLE_EXTENSION_CLIENT_H_
#define WRT_BUNDLE_EXTENSION_CLIENT_H_

#include <string>
#include <memory>
#include <map>
#include <vector>

#include "common/dbus_client.h"

namespace wrt {

class ExtensionClient {
 public:
  struct InstanceHandler {
    virtual void HandleMessageFromNative(const std::string& msg) = 0;
   protected:
    ~InstanceHandler() {}
  };

  ExtensionClient();
  virtual ~ExtensionClient();

  std::string CreateInstance(const std::string& extension_name,
                             InstanceHandler* handler);
  void DestroyInstance(const std::string& instance_id);

  void PostMessageToNative(const std::string& instance_id,
                           const std::string& msg);
  std::string SendSyncMessageToNative(const std::string& instance_id,
                                      const std::string& msg);

  bool Initialize(const std::string& uuid);

  struct ExtensionCodePoints {
    std::string api;
    std::vector<std::string> entry_points;
  };

  typedef std::map<std::string, ExtensionCodePoints*> ExtensionAPIMap;
  const ExtensionAPIMap& extension_apis() const { return extension_apis_; }

 private:
  void HandleSignal(const std::string& signal_name, GVariant* parameters);

  ExtensionAPIMap extension_apis_;
  std::map<std::string, InstanceHandler*> handlers_;
  DBusClient dbus_extension_client_;
};

}  // namespace wrt

#endif  // WRT_BUNDLE_EXTENSION_CLIENT_H_
