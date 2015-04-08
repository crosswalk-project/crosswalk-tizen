// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_VIBRATION_MANAGER_H_
#define WRT_RUNTIME_VIBRATION_MANAGER_H_

// TODO(sngn.lee): this class will move to src/platform/ directory
namespace wrt {
namespace platform {
class VibrationManager {
 public:
  static VibrationManager* GetInstance();
  virtual void Start(int ms) = 0;
  virtual void Stop() = 0;
};
}  // namespace platform
}  // namespace wrt

#endif  // WRT_RUNTIME_VIBRATION_MANAGER_H_
