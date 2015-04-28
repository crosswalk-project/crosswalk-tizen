// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/resource_manager.h"

#include <stdio.h>
#include <aul.h>

#include <memory>
#include <regex>
#include <vector>

#include "common/logger.h"
#include "common/file_utils.h"
#include "common/string_utils.h"
#include "runtime/app_control.h"
#include "runtime/application_data.h"
#include "runtime/locale_manager.h"

using wgt::parse::AppControlInfo;

namespace wrt {

namespace {

typedef std::vector<AppControlInfo> AppControlList;

// Scheme type
const char* kSchemeTypeApp = "app://";
const char* kSchemeTypeFile = "file://";
const char* kSchemeTypeHttp = "http://";
const char* kSchemeTypeHttps = "https://";
const char* kSchemeTypeWidget = "widget://";

// Default Start Files
const char* kDefaultStartFiles[] = {
  "index.htm",
  "index.html",
  "index.svg",
  "index.xhtml",
  "index.xht"
};

static std::string GetMimeFromUri(const std::string& uri) {
  // checking passed uri is local file
  std::string file_uri_case(kSchemeTypeFile);
  int ret = AUL_R_EINVAL;
  char mimetype[128] = {0, };
  int pos = std::string::npos;
  if (utils::StartsWith(uri, file_uri_case)) {
    // case 1. uri = file:///xxxx
    ret = aul_get_mime_from_file(uri.substr(pos+file_uri_case.length()).c_str(),
                                 mimetype, sizeof(mimetype));
  } else if (utils::StartsWith(uri, "/")) {
    // case 2. uri = /xxxx
    ret = aul_get_mime_from_file(uri.c_str(),
                                 mimetype, sizeof(mimetype));
  }

  if (ret == AUL_R_OK) {
    return std::string(mimetype);
  } else {
    return std::string();
  }
}

static bool CompareStringWithWildcard(const std::string& origin,
                                      const std::string& target) {
  std::string wildcard_str = utils::ReplaceAll(origin, "*", ".*");
  try {
    std::regex re(wildcard_str, std::regex_constants::icase);
    return std::regex_match(target.begin(), target.end(), re);
  } catch (std::regex_error& e) {
    LoggerE("regex exception happened");
    return false;
  }
}

static bool CompareMime(const std::string& origin, const std::string& target) {
  return CompareStringWithWildcard(origin, target);
}

static bool CompareUri(const std::string& origin_uri,
                       const std::string& target_uri) {
  std::string origin_scheme = utils::SchemeName(origin_uri);
  std::string target_scheme = utils::SchemeName(target_uri);

  if (!origin_scheme.empty() && !target_scheme.empty()) {
    return (origin_scheme == target_scheme);
  } else {
    return CompareStringWithWildcard(origin_uri, target_uri);
  }
}

static AppControlList::const_iterator CompareMimeAndUri(
    const AppControlList& operation_list,
    const std::string& mime, const std::string& uri) {
  if (mime.empty() && uri.empty()) {
    // 1. request_mime = "", request_uri = ""
    for (auto iter = operation_list.begin();
         iter != operation_list.end(); ++iter) {
      if (iter->mime().empty() && iter->uri().empty()) {
        return iter;
      }
    }
  } else if (mime.empty() && !uri.empty()) {
    // 2.. request_mime = "", request_uri = "blahblah"
    for (auto iter = operation_list.begin();
         iter != operation_list.end(); ++iter) {
      if (iter->mime().empty() && CompareUri(iter->uri(), uri)) {
        return iter;
      }
    }
  } else if (!mime.empty() && uri.empty()) {
    // 3... request_mime = "blahblah", request_uri = ""
    for (auto iter = operation_list.begin();
         iter != operation_list.end(); ++iter) {
      if (iter->uri().empty() && CompareMime(iter->mime(), mime)) {
        return iter;
      }
    }
  } else {
    // 4... request_mime = "blahblah", request_uri = "blahblah"
    for (auto iter = operation_list.begin();
         iter != operation_list.end(); ++iter) {
      if (CompareMime(iter->mime(), mime) &&
          CompareUri(iter->uri(), uri)) {
        return iter;
      }
    }
  }
  return operation_list.end();
}

static void FindOperations(AppControlList* app_control_list,
                           const std::string& operation) {
  auto iter = app_control_list->begin();
  while (iter != app_control_list->end()) {
    if (iter->operation() != operation) {
      app_control_list->erase(iter++);
    } else {
      ++iter;
    }
  }
}

static std::string InsertPrefixPath(const std::string& start_uri) {
  if (start_uri.find("://") != std::string::npos) {
    return start_uri;
  } else {
    return std::string() + kSchemeTypeFile + "/" + start_uri;
  }
}

}  // namespace

ResourceManager::ResourceManager(ApplicationData* application_data,
                                 LocaleManager* locale_manager)
    : application_data_(application_data), locale_manager_(locale_manager) {
  if (application_data != NULL) {
    appid_ = application_data->tizen_application_info()->id();
  }
}

std::string ResourceManager::GetDefaultOrEmpty() {
  using wgt::parse::AppWidgetVector;
  std::string default_src;

  // TODO(yons.kim): tizen content src

  // content src
  const AppWidgetVector app_widgets =
    application_data_->app_widget_info()->app_widgets();
  for (auto iter = app_widgets.begin();
       iter != app_widgets.end(); ++iter) {
    if (iter->id == appid_) {
      default_src = iter->content_src;
      break;
    }
  }
  if (!default_src.empty()) {
    return InsertPrefixPath(default_src);
  }

  // if there is no content src, find reserved index files
  for (auto& start_file : kDefaultStartFiles) {
    if (utils::Exists(resource_base_path_ + start_file)) {
      default_src = start_file;
    }
  }

  return InsertPrefixPath(default_src);
}

std::string ResourceManager::GetMatchedSrcOrUri(
    const AppControlInfo& app_control_info) {
  if (!app_control_info.src().empty()) {
    return InsertPrefixPath(app_control_info.src());
  }

  if (!app_control_info.uri().empty()) {
    return InsertPrefixPath(app_control_info.uri());
  }

  return GetDefaultOrEmpty();
}

std::string ResourceManager::GetStartURL() {
  std::string operation = app_control_->operation();
  if (operation.empty()) {
    LoggerE("operation(mandatory) is NULL");
    return GetDefaultOrEmpty();
  }

  std::string mime = app_control_->mime();
  std::string uri = app_control_->uri();
  if (mime.empty() && !uri.empty()) {
    mime = GetMimeFromUri(uri);
  }

  LoggerD("Passed AppControl data");
  LoggerD(" - operation : %s", operation.c_str());
  LoggerD(" - mimetype  : %s", mime.c_str());
  LoggerD(" - uri       : %s", uri.c_str());

  if (application_data_ == NULL ||
      application_data_->app_control_info_list() == NULL) {
    return GetDefaultOrEmpty();
  }

  AppControlList app_control_list =
    application_data_->app_control_info_list()->controls;
  FindOperations(&app_control_list, operation);

  if (!app_control_list.empty()) {
    AppControlList::const_iterator iter =
      CompareMimeAndUri(app_control_list, mime, uri);
    if (iter != app_control_list.end()) {
      return GetMatchedSrcOrUri(*iter);
    } else {
      return GetDefaultOrEmpty();
    }
  } else {
    return GetDefaultOrEmpty();
  }
}

std::string ResourceManager::GetLocalizedPath(const std::string& origin) {
  std::string file_scheme = std::string() + kSchemeTypeFile + "/";
  std::string app_scheme = std::string() + kSchemeTypeApp;
  std::string locale_path = "locales/";
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

  if (url.compare(app_scheme) == 0) {
    // remove "app://"
    url.erase(0, app_scheme.length());

    // remove app id + /
    std::string check = appid_ + "/";
    if (url.compare(0, check.length(), check) == 0) {
      url.erase(0, check.length());
    } else {
      LoggerE("appid was invalid");
      return result;
    }
  } else if (url.compare(file_scheme) == 0) {
    // remove "file:///"
    url.erase(0, file_scheme.length());
  }

  if (!url.empty() && url[url.length()-1] == '/') {
      url.erase(url.length()-1, 1);
  }

  if (url.empty()) {
    LoggerE("URL Localization error");
    return result;
  }

  auto locales = locale_manager_->system_locales().begin();
  for ( ; locales != locale_manager_->system_locales().end(); ++locales) {
    // check ../locales/
    std::string app_locale_path = resource_base_path_ + locale_path;
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

void ResourceManager::set_base_resource_path(const std::string& path) {
  if (path.empty()) {
    return;
  }

  resource_base_path_ = path;
  if (resource_base_path_[resource_base_path_.length()-1] != '/') {
    resource_base_path_ += "/";
  }
}

bool ResourceManager::Exists(const std::string& path) {
  auto find = file_existed_cache_.find(path);
  if (find != file_existed_cache_.end()) {
    return find->second;
  }
  bool ret = file_existed_cache_[path] = utils::Exists(path);
  return ret;
}

}  // namespace wrt
