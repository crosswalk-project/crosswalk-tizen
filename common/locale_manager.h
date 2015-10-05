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

#ifndef XWALK_COMMON_LOCALE_MANAGER_H_
#define XWALK_COMMON_LOCALE_MANAGER_H_

#include <list>
#include <map>
#include <string>

namespace common {

class LocaleManager {
 public:
  typedef std::map<std::string, std::string> StringMap;

  LocaleManager();
  virtual ~LocaleManager();
  void SetDefaultLocale(const std::string& locale);
  void EnableAutoUpdate(bool enable);
  void UpdateSystemLocale();
  const std::list<std::string>& system_locales() const
    { return system_locales_; }

  std::string GetLocalizedString(const StringMap& strmap);

 private:
  std::string default_locale_;
  std::list<std::string> system_locales_;
};

}  // namespace common

#endif  // XWALK_COMMON_LOCALE_MANAGER_H_
