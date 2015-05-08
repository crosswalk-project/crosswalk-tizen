// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/vibration_manager.h"

#include <dd-haptic.h>

#include "common/logger.h"


namespace wrt {
namespace platform {

class VibrationImpl : public VibrationManager {
 public:
  VibrationImpl();
  virtual ~VibrationImpl();
  virtual void Start(int ms);
  virtual void Stop();
 private:
  bool Initialize();
  // haptic_devce_h was declared as int
  haptic_device_h handle_;
};


VibrationImpl::VibrationImpl()
    : handle_(0) {
}

VibrationImpl::~VibrationImpl() {
  if (handle_ != 0) {
    haptic_close(handle_);
    handle_ = 0;
  }
}

bool VibrationImpl::Initialize() {
  if (handle_ != 0)
    return true;

  int ret = haptic_open(HAPTIC_DEVICE_0, &handle_);
  if (ret != HAPTIC_ERROR_NONE) {
    LOGGER(ERROR) << "Fail to open haptic device";
    handle_ = 0;
    return false;
  }
  return true;
}

void VibrationImpl::Start(int ms) {
  if (Initialize()) {
    haptic_vibrate_monotone(handle_, ms, NULL);
  }
}

void VibrationImpl::Stop() {
  if (Initialize()) {
    haptic_stop_all_effects(handle_);
  }
}

VibrationManager* VibrationManager::GetInstance() {
  static VibrationImpl instance;
  return &instance;
}


}  // namespace platform
}  // namespace wrt


