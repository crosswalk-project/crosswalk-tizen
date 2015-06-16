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

#ifndef WRT_BUNDLE_WIDGET_MODULE_H_
#define WRT_BUNDLE_WIDGET_MODULE_H_

#include <list>
#include <string>
#include "bundle/module_system.h"

namespace wrt {
class ApplicationData;
class LocaleManager;

// This module provides widget object
class WidgetModule : public NativeModule {
 public:
  WidgetModule();
  ~WidgetModule() override;

 private:
  v8::Handle<v8::Object> NewInstance() override;
  v8::Persistent<v8::ObjectTemplate> preference_object_template_;
};

class WidgetPreferenceDB {
 public:
  static WidgetPreferenceDB* GetInstance();
  void Initialize(const ApplicationData* appdata,
                  LocaleManager* locale_manager);
  void InitializeDB();
  int Length();
  bool Key(int idx, std::string* key);
  bool GetItem(const std::string& key, std::string* value);
  bool SetItem(const std::string& key, const std::string& value);
  bool RemoveItem(const std::string& key);
  bool HasItem(const std::string& key);
  void Clear();
  void GetKeys(std::list<std::string>* keys);

  std::string author();
  std::string description();
  std::string name();
  std::string shortName();
  std::string version();
  std::string id();
  std::string authorEmail();
  std::string authorHref();
  unsigned int height();
  unsigned int width();

 private:
  WidgetPreferenceDB();
  virtual ~WidgetPreferenceDB();
  const ApplicationData* appdata_;
  LocaleManager* locale_manager_;
};
}  // namespace wrt

#endif  // WRT_BUNDLE_WIDGET_MODULE_H_
