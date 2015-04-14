// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_RESOURCE_MANAGER_H_
#define WRT_RUNTIME_RESOURCE_MANAGER_H_

#include <string>
#include <map>

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
  ResourceManager(ApplicationData* application_data,
                  LocaleManager* locale_manager);
  ~ResourceManager() {}

  // input : file:///..... , app://[appid]/....
  // output : /[system path]/.../locales/.../
  std::string GetLocalizedPath(const std::string& origin);
  std::string GetStartURL();

  void set_base_resource_path(const std::string& base_path);
  void set_app_control(AppControl* app_control) {
    app_control_ = app_control;
  }

  const AppControl* app_control() const { return app_control_; }

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
  AppControl* app_control_;
};

}  // namespace wrt

#endif  // WRT_RUNTIME_RESOURCE_MANAGER_H_
