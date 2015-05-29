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

#ifndef WRT_COMMON_RESOURCE_MANAGER_H_
#define WRT_COMMON_RESOURCE_MANAGER_H_

#include <string>
#include <map>
#include <memory>

namespace wgt {

namespace parse {

class AppControlInfo;

}  // namespace parse

}  // namespace wgt

namespace wrt {

class ApplicationData;
class LocaleManager;
class AppControl;

class ResourceManager {
 public:
  class Resource {
   public:
    explicit Resource(const std::string& uri);
    Resource(const std::string& uri, const std::string& mime);
    Resource(const std::string& uri, const std::string& mime,
             bool should_reset);
    Resource(const Resource& res);
    ~Resource() {}

    Resource& operator=(const Resource& res);
    bool operator==(const Resource& res);

    void set_uri(const std::string& uri) { uri_ = uri; }
    void set_mime(const std::string& mime) { mime_ = mime; }
    void set_should_reset(bool should_reset) { should_reset_ = should_reset; }

    std::string uri() const { return uri_; }
    std::string mime() const { return mime_; }
    bool should_reset() const { return should_reset_; }

   private:
    std::string uri_;
    std::string mime_;
    bool should_reset_;
  };

  ResourceManager(ApplicationData* application_data,
                  LocaleManager* locale_manager);
  ~ResourceManager() {}

  // input : file:///..... , app://[appid]/....
  // output : /[system path]/.../locales/.../
  std::string GetLocalizedPath(const std::string& origin);
  std::unique_ptr<Resource> GetStartResource(const AppControl* app_control);

  void set_base_resource_path(const std::string& base_path);

 private:
  std::string GetMatchedSrcOrUri(const wgt::parse::AppControlInfo&);
  std::string GetDefaultOrEmpty();
  // for localization
  bool Exists(const std::string& path);

  std::string resource_base_path_;
  std::string appid_;
  std::map<const std::string, bool> file_existed_cache_;
  std::map<const std::string, std::string> locale_cache_;

  ApplicationData* application_data_;
  LocaleManager* locale_manager_;
};

}  // namespace wrt

#endif  // WRT_COMMON_RESOURCE_MANAGER_H_
