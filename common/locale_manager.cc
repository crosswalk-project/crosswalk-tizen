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

#include "common/locale_manager.h"

#include <system_settings.h>

#include <algorithm>
#include <memory>

#include "common/file_utils.h"
#include "common/logger.h"

namespace common {

namespace {

std::string localeToBCP47LangTag(
    const std::string locale) {
  // Cut off codepage information from given string (if any exists)
  // i.e. change en_US.UTF-8 into en_US */
  std::string lang = locale.substr(0, locale.find_first_of("."));

  // Replace all '_' with '-'
  std::replace(lang.begin(), lang.end(), '_', '-');
  return lang;
}

}  // namespace


LocaleManager::LocaleManager() {
  UpdateSystemLocale();
}

LocaleManager::~LocaleManager() {
  system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE);
}

void LocaleManager::EnableAutoUpdate(bool enable) {
  if (enable) {
    auto callback = [](system_settings_key_e, void* user_data) {
        LocaleManager* locale = static_cast<LocaleManager*>(user_data);
        locale->UpdateSystemLocale();
    };
    system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
                                   callback, this);
  } else {
    system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE);
  }
}

void LocaleManager::SetDefaultLocale(const std::string& locale) {
  if (!default_locale_.empty() && system_locales_.size() > 0 &&
       system_locales_.back() == default_locale_) {
    system_locales_.pop_back();
  }
  default_locale_ = locale;
  if (!default_locale_.empty()) {
    system_locales_.push_back(locale);
  }
}

void LocaleManager::UpdateSystemLocale() {
  char* str = NULL;
  if (SYSTEM_SETTINGS_ERROR_NONE !=
      system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
                                       &str) ||
      str == NULL) {
    return;
  }
  std::string lang = localeToBCP47LangTag(str);
  free(str);

  if (lang.length() == 0) {
    LOGGER(ERROR) << "Language tag was invalid";
    return;
  }

  system_locales_.clear();
  while (true) {
    LOGGER(DEBUG) << "Processing language description: " << lang;
    system_locales_.push_back(lang);

    // compatibility with lower language Tag by SDK
    std::string lower = lang;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower != lang) {
      system_locales_.push_back(lower);
    }
    size_t position = lang.find_last_of("-");
    if (position == std::string::npos) {
      break;
    }
    lang = lang.substr(0, position);
  }
  if (!default_locale_.empty()) {
    system_locales_.push_back(default_locale_);
  }
}

std::string LocaleManager::GetLocalizedString(const StringMap& strmap) {
  if (strmap.empty()) {
    return std::string();
  }

  // find string with system locales
  for (auto& locale : system_locales_) {
    auto it = strmap.find(locale);
    if (it != strmap.end()) {
      return it->second;
    }
  }

  // find string with empty locale
  auto it = strmap.find("");
  if (it != strmap.end()) {
    return it->second;
  }

  // If localized string is not found, return first string.
  return strmap.begin()->second;
}

}  // namespace common
