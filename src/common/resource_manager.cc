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
#include <vector>
#include <functional>
#include <algorithm>

#include "common/logger.h"
#include "common/file_utils.h"
#include "common/string_utils.h"
#include "common/app_control.h"
#include "common/application_data.h"
#include "common/locale_manager.h"
#include "common/url.h"

using wgt::parse::AppControlInfo;

namespace wrt {

namespace {

typedef std::vector<AppControlInfo> AppControlList;

// Scheme type
const char* kSchemeTypeApp = "app://";
const char* kSchemeTypeFile = "file://";
const char* kSchemeTypeHttp = "http://";
const char* kSchemeTypeHttps = "https://";
// lendth of scheme identifier ://
const int kSchemeIdLen = 3;
// TODO(wy80.choi): comment out below unused const variables if needed.
// const char* kSchemeTypeWidget = "widget://";

// Default Start Files
const char* kDefaultStartFiles[] = {
  "index.htm",
  "index.html",
  "index.svg",
  "index.xhtml",
  "index.xht"
};

// Default Encoding
const char* kDefaultEncoding = "UTF-8";

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

static bool CompareMime(const std::string& info_mime,
                        const std::string& request_mime) {
  if (request_mime.empty())
    return info_mime.empty();

  // suppose that these mimetypes are valid expressions ('type'/'sub-type')
  if (info_mime == "*" || info_mime == "*/*")
    return true;

  std::string info_type;
  std::string info_sub;
  std::string request_type;
  std::string request_sub;
  if (!(utils::SplitString(info_mime, &info_type, &info_sub, '/') &&
        utils::SplitString(request_mime, &request_type, &request_sub, '/')))
    return false;

  return info_type == request_type &&
         (info_sub == "*") ? true : (info_sub == request_sub);
}

static bool CompareUri(const std::string& info_uri,
                       const std::string& request_uri) {
  if (request_uri.empty())
    return info_uri.empty();

  std::string info_scheme = utils::SchemeName(info_uri);

  // if has only scheme or scheme+star. ex) http, http://, http://*
  if (!info_scheme.empty() &&
      (info_uri == info_scheme || utils::EndsWith(info_uri, "://")
        || utils::EndsWith(info_uri, "://*"))) {
    return utils::SchemeName(request_uri) == info_scheme;
  }

  if (utils::EndsWith(info_uri, "*")) {
    return utils::StartsWith(request_uri, info_uri.substr(0,
                                                          info_uri.length()-1));
  } else {
    return request_uri == info_uri;
  }
}

static std::string InsertPrefixPath(const std::string& start_uri) {
  if (start_uri.find("://") != std::string::npos)
    return start_uri;
  else
    return std::string(kSchemeTypeFile) + "/" + start_uri;
}

}  // namespace

ResourceManager::Resource::Resource(const std::string& uri)
  : uri_(uri), should_reset_(true), encoding_(kDefaultEncoding) {}

ResourceManager::Resource::Resource(const std::string& uri, bool should_reset)
  : uri_(uri), should_reset_(should_reset), encoding_(kDefaultEncoding) {}

ResourceManager::Resource::Resource(const std::string& uri,
                                    const std::string& mime,
                                    const std::string& encoding)
  : uri_(uri), mime_(mime), should_reset_(true), encoding_(encoding) {}

ResourceManager::Resource::Resource(const ResourceManager::Resource& res) {
  *this = res;
}

ResourceManager::Resource& ResourceManager::Resource::operator=(
    const ResourceManager::Resource& res) {
  this->uri_ = res.uri();
  this->mime_ = res.mime();
  this->should_reset_ = res.should_reset();
  this->encoding_ = res.encoding();
  return *this;
}

bool ResourceManager::Resource::operator==(const Resource& res) {
  if (this->uri_ == res.uri() && this->mime_ == res.mime()
     && this->should_reset_ == res.should_reset()
     && this->encoding_ == res.encoding()) {
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
    if (application_data->csp_info() != NULL ||
        application_data->csp_report_info() != NULL ||
        application_data->allowed_navigation_info() != NULL) {
      security_model_version_ = 2;
    } else {
      security_model_version_ = 1;
    }
  }
}

std::unique_ptr<ResourceManager::Resource>
ResourceManager::GetDefaultResource() {
  std::string default_src;
  std::string type;
  std::string encoding = kDefaultEncoding;

  auto content_info = application_data_->content_info();
  if (content_info) {
    default_src = content_info->src();
    // TODO(yons.kim): uncomment below codes after implementing
    //                 content info handler
    // type = content_info->type();
    // encoding = (!content_info->encoding().empty())
    //            ? content_info->encoding() : kDefaultEncoding;
  }

  if (!default_src.empty()) {
    default_src = InsertPrefixPath(default_src);
  } else {
    // if there is no content src, find reserved index files
    for (auto& start_file : kDefaultStartFiles) {
      if (utils::Exists(resource_base_path_ + start_file)) {
        default_src = InsertPrefixPath(start_file);
        break;
      }
    }
  }

  return std::unique_ptr<Resource>(new Resource(default_src, type, encoding));
}

std::unique_ptr<ResourceManager::Resource> ResourceManager::GetMatchedResource(
    const AppControlInfo& app_control_info) {
  if (!app_control_info.src().empty()) {
    return std::unique_ptr<Resource>(new Resource(
      InsertPrefixPath(app_control_info.src()),
                       app_control_info.onreset() == "disable" ? false : true));
  }

  return GetDefaultResource();
}

std::unique_ptr<ResourceManager::Resource> ResourceManager::GetStartResource(
    const AppControl* app_control) {
  std::string operation = app_control->operation();
  if (operation.empty()) {
    LOGGER(ERROR) << "operation(mandatory) is NULL";
    return GetDefaultResource();
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
    return GetDefaultResource();
  }

  auto app_control_list =
    application_data_->app_control_info_list()->controls;

  AppControlList::const_iterator iter = std::find_if(
    app_control_list.begin(), app_control_list.end(),
    [&operation, &uri, &mime](AppControlInfo& info) -> bool {
      return (info.operation() == operation)
        && CompareMime(info.mime(), mime) && CompareUri(info.uri(), uri); });

  if (iter != app_control_list.end()) {
    return GetMatchedResource(*iter);
  } else {
  return GetDefaultResource();
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

    // check locale path ../locales/en_us/
    std::string app_localized_path = app_locale_path + locales + "/";
    if (!Exists(app_localized_path)) {
      continue;
    }
    std::string resource_path = app_localized_path + url;
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

  LOGGER(ERROR) << "URL Localization error";
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

bool ResourceManager::AllowNavigation(const std::string& url) {
  if (security_model_version_ == 2)
    return CheckAllowNavigation(url);
  return CheckWARP(url);
}

bool ResourceManager::AllowedResource(const std::string& url) {
  if (security_model_version_ == 2)
    return true;
  return CheckWARP(url);
}

bool ResourceManager::CheckWARP(const std::string& url) {
  // allow non-external resource
  if (!utils::StartsWith(url, kSchemeTypeHttp) &&
      !utils::StartsWith(url, kSchemeTypeHttps)) {
    return true;
  }

  auto warp = application_data_->warp_info();
  if (warp.get() == NULL)
    return false;

  auto find = warp_cache_.find(url);
  if (find != warp_cache_.end()) {
    return find->second;
  }

  bool& result = warp_cache_[url];
  result = true;

  URL url_info(url);

  // if didn't have a scheme, it means local resource
  if (url_info.scheme().empty()) {
    return true;
  }

  for (auto& allow : warp->access_map()) {
    if (allow.first == "*") {
      return true;
    } else if (allow.first.empty()) {
      continue;
    }

    URL allow_url(allow.first);

    // should be match the scheme and port
    if (allow_url.scheme() != url_info.scheme() ||
        allow_url.port() != url_info.port()) {
      continue;
    }

    // if domain alos was matched, allow resource
    if (allow_url.domain() == url_info.domain()) {
      return true;
    } else if (allow.second) {
      // if does not match domain, should be check sub domain

      // filter : test.com , subdomain=true
      // url : aaa.test.com
      // check url was ends with ".test.com"
      if (utils::EndsWith(url_info.domain(), "." + allow_url.domain())) {
        return true;
      }
    }
  }

  return result = false;
}

bool ResourceManager::CheckAllowNavigation(const std::string& url) {
  // allow non-external resource
  if (!utils::StartsWith(url, kSchemeTypeHttp) &&
      !utils::StartsWith(url, kSchemeTypeHttps)) {
    return true;
  }

  auto allow = application_data_->allowed_navigation_info();
  if (allow.get() == NULL)
    return false;

  auto find = warp_cache_.find(url);
  if (find != warp_cache_.end()) {
    return find->second;
  }

  bool& result = warp_cache_[url];
  result = true;

  URL url_info(url);

  // if didn't have a scheme, it means local resource
  if (url_info.scheme().empty()) {
    return true;
  }

  for (auto& allow_domain : allow->GetAllowedDomains()) {
    URL a_domain_info(allow_domain);

    // check wildcard *
    if (a_domain_info.domain() == "*") {
      return true;
    }

    bool prefix_wild = false;
    bool suffix_wild = false;
    std::string a_domain = a_domain_info.domain();
    if (utils::StartsWith(a_domain, "*.")) {
      prefix_wild = true;
      // *.domain.com -> .domain.com
      a_domain = a_domain.substr(1);
    }
    if (utils::EndsWith(a_domain, ".*")) {
      suffix_wild = true;
      // domain.* -> domain.
      a_domain = a_domain.substr(0, a_domain.length() - 1);
    }

    if (!prefix_wild && !suffix_wild) {
      // if no wildcard, should be exactly matched
      if (url_info.domain() == a_domain) {
        return true;
      }
    } else if (prefix_wild && !suffix_wild) {
      // *.domain.com : it shoud be "domain.com" or end with ".domain.com"
      if (url_info.domain() == a_domain.substr(1) ||
          utils::EndsWith(url_info.domain(), a_domain)) {
        return true;
      }
    } else if (!prefix_wild && suffix_wild) {
      // www.sample.* : it should be starts with "www.sample."
      if (utils::StartsWith(url_info.domain(), a_domain)) {
        return true;
      }
    } else if (prefix_wild && suffix_wild) {
      // *.sample.* : it should be starts with sample. or can find ".sample."
      // in url
      if (utils::StartsWith(url_info.domain(), a_domain.substr(1)) ||
          std::string::npos != url_info.domain().find(a_domain)) {
        return true;
      }
    }
  }

  return result = false;
}

}  // namespace wrt
