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

#include "runtime/browser/notification_manager.h"

#include <notification.h>
#include <notification_internal.h>

#include <memory>

#include "common/logger.h"

namespace runtime {

NotificationManager* NotificationManager::GetInstance() {
  static NotificationManager instance;
  return &instance;
}

NotificationManager::NotificationManager() {
}

bool NotificationManager::Show(uint64_t tag,
                               const std::string& title,
                               const std::string& body,
                               const std::string& icon_path) {
  auto found = keymapper_.find(tag);
  if (found != keymapper_.end()) {
    Hide(tag);
  }

  notification_h noti_h = NULL;
  int ret = NOTIFICATION_ERROR_NONE;
  noti_h = notification_new(
      NOTIFICATION_TYPE_NOTI,
      NOTIFICATION_GROUP_ID_DEFAULT,
      NOTIFICATION_PRIV_ID_NONE);
  if (noti_h == NULL) {
    LOGGER(ERROR) << "Can't create notification handle";
    return false;
  }

  std::unique_ptr<std::remove_pointer<notification_h>::type,
                  decltype(notification_free)*>
      auto_release {noti_h, notification_free};

  // set notification title
  ret = notification_set_text(
      noti_h,
      NOTIFICATION_TEXT_TYPE_TITLE,
      title.c_str(),
      NULL,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LOGGER(ERROR) << "Can't set title";
    return false;
  }

  // set notification content
  ret = notification_set_text(
      noti_h,
      NOTIFICATION_TEXT_TYPE_CONTENT,
      body.c_str(),
      NULL,
      NOTIFICATION_VARIABLE_TYPE_NONE);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LOGGER(ERROR) << "Can't set content";
    return false;
  }

  if (!icon_path.empty()) {
    ret = notification_set_image(
        noti_h,
        NOTIFICATION_IMAGE_TYPE_ICON,
        icon_path.c_str());
    if (ret != NOTIFICATION_ERROR_NONE) {
      LOGGER(ERROR) << "Can't set icon";
      return false;
    }
  }

  // insert notification
  int platform_key = NOTIFICATION_PRIV_ID_NONE;
  ret = notification_insert(noti_h, &platform_key);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LOGGER(ERROR) << "Can't insert notification";
    return false;
  }
  keymapper_[tag] = platform_key;
  return true;
}

bool NotificationManager::Hide(uint64_t tag) {
  auto found = keymapper_.find(tag);
  if (found == keymapper_.end()) {
    LOGGER(ERROR) << "Can't find notification";
    return false;
  }
  notification_delete_by_priv_id(NULL,
                                 NOTIFICATION_TYPE_NOTI,
                                 found->second);
  keymapper_.erase(found);
  return true;
}

}  // namespace runtime
