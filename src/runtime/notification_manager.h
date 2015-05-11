// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_NOTIFICATION_MANAGER_H_
#define WRT_RUNTIME_NOTIFICATION_MANAGER_H_

#include <string>
#include <map>

namespace wrt {
class NotificationManager {
 public:
  static NotificationManager* GetInstance();
  bool Show(uint64_t tag,
            const std::string& title,
            const std::string& body,
            const std::string& icon_path);
  bool Hide(uint64_t tag);
 private:
  NotificationManager();
  std::map<uint64_t, int> keymapper_;
};
}  // namespace wrt

#endif   // WRT_RUNTIME_NOTIFICATION_MANAGER_H_
