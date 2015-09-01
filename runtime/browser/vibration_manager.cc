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

#include "runtime/browser/vibration_manager.h"

#include <dd-haptic.h>

#include "common/logger.h"

namespace runtime {
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
}  // namespace runtime
