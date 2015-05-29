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

#include "common/resource_manager.h"

#include <stdio.h>
#include <aul.h>

#include <memory>
#include <regex>
#include <vector>

#include "common/logger.h"
#include "common/file_utils.h"
#include "common/string_utils.h"
#include "common/app_control.h"
#include "common/application_data.h"
#include "common/locale_manager.h"

using wgt::parse::AppControlInfo;

namespace wrt {

namespace {

typedef std::vector<AppControlInfo> AppControlList;

// Scheme type
const char* kSchemeTypeApp = "app://";
const char* kSchemeTypeFile = "file://";
// TODO(wy80.choi): comment out below unused const variables if needed.
// const char* kSchemeTypeHttp = "http://";
// const char* kSchemeTypeHttps = "https://";
// const char* kSchemeTypeWidget = "widget://";

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
  size_t pos = std::string::npos;
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
    LOGGER(ERROR) << "regex_error caught: " << e.what();
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

ResourceManager::Resource::Resource(const std::string& uri)
  : uri_(uri), should_reset_(true) {}

ResourceManager::Resource::Resource(const std::string& uri,
                                    const std::string& mime)
  : uri_(uri), mime_(mime), should_reset_(true) {}

ResourceManager::Resource::Resource(const std::string& uri,
                                    const std::string& mime, bool should_reset)
  : uri_(uri), mime_(mime), should_reset_(should_reset) {}

ResourceManager::Resource::Resource(const ResourceManager::Resource& res) {
  *this = res;
}

ResourceManager::Resource& ResourceManager::Resource::operator=(
    const ResourceManager::Resource& res) {
  this->uri_ = res.uri();
  this->mime_ = res.mime();
  this->should_reset_ = res.should_reset();
  return *this;
}

bool ResourceManager::Resource::operator==(const Resource& res) {
  if (this->uri_ == res.uri() && this->mime_ == res.mime()
     && this->should_reset_ == res.should_reset()) {
    return true;
  } else {
    return false;
  }
}

ResourceManager::ResourceManager(ApplicationData* application_data,
                                 LocaleManager* locale_manager)
    : application_data_(application_data), locale_manager_(locale_manager) {
  if (application_data != NULL) {
    appid_ = application_data->tizen_application_info()->id();
  }
}

std::string ResourceManager::GetDefaultOrEmpty() {
  std::string default_src;

  // content src
  auto content_info = application_data_->content_info();
  if (content_info) {
    default_src = content_info->src();
  } else {
    LOGGER(WARN) << "ContentInfo is NULL.";
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

std::unique_ptr<ResourceManager::Resource> ResourceManager::GetStartResource(
    const AppControl* app_control) {
  std::string operation = app_control->operation();
  if (operation.empty()) {
    LOGGER(ERROR) << "operation(mandatory) is NULL";
    return std::unique_ptr<Resource>(new Resource(GetDefaultOrEmpty()));
  }

  std::string mime = app_control->mime();
  std::string uri = app_control->uri();
  if (mime.empty() && !uri.empty()) {
    mime = GetMimeFromUri(uri);
  }

  LOGGER(DEBUG) << "Passed AppControl data";
  LOGGER(DEBUG) << " - operation : " << operation;
  LOGGER(DEBUG) << " - mimetype  : " << mime;
  LOGGER(DEBUG) << " - uri       : " << uri;

  if (application_data_ == NULL ||
      application_data_->app_control_info_list() == NULL) {
    return std::unique_ptr<Resource>(new Resource(GetDefaultOrEmpty()));
  }

  AppControlList app_control_list =
    application_data_->app_control_info_list()->controls;
  FindOperations(&app_control_list, operation);

  if (!app_control_list.empty()) {
    AppControlList::const_iterator iter =
      CompareMimeAndUri(app_control_list, mime, uri);
    if (iter != app_control_list.end()) {
      // TODO(jh5.cho) : following comment will be added after SRPOL implement
      return std::unique_ptr<Resource>(
        new Resource(GetMatchedSrcOrUri(*iter), iter->mime()
                   /*, iter->should_reset()*/));
    } else {
    return std::unique_ptr<Resource>(new Resource(GetDefaultOrEmpty()));
    }
  } else {
    return std::unique_ptr<Resource>(new Resource(GetDefaultOrEmpty()));
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

  if (utils::StartsWith(url, app_scheme)) {
    // remove "app://"
    url.erase(0, app_scheme.length());

    // remove app id + /
    std::string check = appid_ + "/";
    if (utils::StartsWith(url, check)) {
      url.erase(0, check.length());
    } else {
      LOGGER(ERROR) << "Invalid appid";
      return result;
    }
  } else if (utils::StartsWith(url, file_scheme)) {
    // remove "file:///"
    url.erase(0, file_scheme.length());
  }

  if (!url.empty() && url[url.length()-1] == '/') {
      url.erase(url.length()-1, 1);
  }

  if (url.empty()) {
    LOGGER(ERROR) << "URL Localization error";
    return result;
  }

  for (auto& locales : locale_manager_->system_locales()) {
    // check ../locales/
    std::string app_locale_path = resource_base_path_ + locale_path;
    if (!Exists(app_locale_path)) {
      break;
    }
    std::string resource_path = app_locale_path + locales + "/" + url;
    if (Exists(resource_path)) {
      result = "file://" + resource_path + suffix;
      return result;
    }
  }

  std::string default_locale = resource_base_path_ + url;
  if (Exists(default_locale)) {
    result = "file://" + default_locale + suffix;
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
