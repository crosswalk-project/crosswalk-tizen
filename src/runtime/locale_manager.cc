// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/locale_manager.h"

#include <runtime_info.h>
#include <memory>
#include <algorithm>

#include "common/logger.h"
#include "common/file_utils.h"

namespace wrt {

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

void LocaleManager::set_base_resource_path(const std::string& path) {
  resource_base_path_ = path;
  if (resource_base_path_[resource_base_path_.length()-1] != '/')
    resource_base_path_ += "/";
}

void LocaleManager::UpdateSystemLocale() {
  char* str = NULL;
  if (RUNTIME_INFO_ERROR_NONE !=
      runtime_info_get_value_string(RUNTIME_INFO_KEY_LANGUAGE, &str)
     || str == NULL) {
    return;
  }
  std::string lang = localeToBCP47LangTag(str);

  if (lang.length() == 0) {
    LoggerE("Language tag was invalid");
    return;
  }

  system_locales_.clear();
  while (true) {
    LoggerD("Processing language description: %s", lang.c_str());
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

bool LocaleManager::Exists(const std::string& path) {
  auto find = file_existed_cache_.find(path);
  if (find != file_existed_cache_.end())
    return find->second;
  return file_existed_cache_[path] = utils::Exists(path);
}

std::string LocaleManager::GetLocalizedPath(const std::string& origin) {
  const char* FILE_SCHEME   = "file:///";
  const char* APP_SCHEME     = "app://";
  const char* LOCALE_PATH   = "locales/";
  auto find = locale_cache_.find(origin);
  if (find != locale_cache_.end()) {
    return find->second;
  }
  std::string& result = locale_cache_[origin];
  std::string url = origin;

  std::string suffix;
  size_t pos = url.find_first_of("#?");
  if (pos != std::string::npos) {
    suffix = url.substr(pos);
    url.resize(pos);
  }

  if (url.compare(0, strlen(APP_SCHEME), APP_SCHEME) == 0) {
    // remove "app://"
    url.erase(0, strlen(APP_SCHEME));

    // remove app id + /
    std::string check = appid_+"/";
    if (url.compare(0, check.length(), check) == 0) {
      url.erase(0, check.length());
    } else {
      LoggerE("appid was invalid");
      return result;
    }
  } else if (url.compare(0, strlen(FILE_SCHEME), FILE_SCHEME) == 0) {
    // remove "file:///"
    url.erase(0, strlen(FILE_SCHEME));
  }

  if (!url.empty() && url[url.length()-1] == '/') {
      url.erase(url.length()-1, 1);
  }

  if (url.empty()) {
    LoggerE("URL Localization error");
    return result;
  }

  auto locales = system_locales_.begin();
  for ( ; locales != system_locales_.end(); ++locales) {
    // check ../locales/
    std::string app_locale_path = resource_base_path_ + LOCALE_PATH;
    if (!Exists(app_locale_path)) {
      break;
    }
    std::string resource_path = app_locale_path + (*locales) + "/" + url;
    if (Exists(resource_path)) {
      result = resource_path + suffix;
      return result;
    }
  }
  std::string default_locale = resource_base_path_ + url;
  if (Exists(default_locale)) {
    result = default_locale + suffix;
    return result;
  }
  result = url + suffix;
  return result;
}



}  // namespace wrt
