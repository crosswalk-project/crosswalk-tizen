// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_LOCALE_MANAGER_H_
#define WRT_RUNTIME_LOCALE_MANAGER_H_

#include <string>
#include <list>
#include <map>

namespace wrt {

class LocaleManager {
 public:
  LocaleManager();
  virtual ~LocaleManager();
  void SetDefaultLocale(const std::string& locale);
  void UpdateSystemLocale();
  const std::list<std::string>& system_locales() const
    { return system_locales_; }

  // for localization, it will be move to another class
  // input : file:///..... , app://[appid]/....
  // output : /[system path]/.../locales/.../
  std::string GetLocalizedPath(const std::string& origin);
  void set_base_resource_path(const std::string& base_path);
  void set_appid(const std::string& appid) { appid_ = appid; }


 private:
  std::string default_locale_;
  std::list<std::string> system_locales_;

  // for localization
  bool Exists(const std::string& path);
  std::string resource_base_path_;
  std::string appid_;
  std::map<const std::string, bool> file_existed_cache_;
  std::map<const std::string, std::string> locale_cache_;
};

}  // namespace wrt


#endif  // WRT_RUNTIME_LOCALE_MANAGER_H_
