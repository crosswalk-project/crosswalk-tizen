// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_LOCALE_MANAGER_H_
#define WRT_COMMON_LOCALE_MANAGER_H_

#include <string>
#include <list>

namespace wrt {

class LocaleManager {
 public:
  LocaleManager();
  virtual ~LocaleManager();
  void SetDefaultLocale(const std::string& locale);
  void EnableAutoUpdate(bool enable);
  void UpdateSystemLocale();
  const std::list<std::string>& system_locales() const
    { return system_locales_; }

 private:
  std::string default_locale_;
  std::list<std::string> system_locales_;
};

}  // namespace wrt

#endif  // WRT_COMMON_LOCALE_MANAGER_H_
