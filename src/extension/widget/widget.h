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

#ifndef WRT_EXTENSION_WIDGET_H_
#define WRT_EXTENSION_WIDGET_H_

#include <string>

namespace wrt {
class ApplicationData;
class Widget {
 public:
  static Widget* GetInstance();
  void Initialize(const ApplicationData* appdata);
  int Length();
  bool Key(int idx, std::string* key);
  bool GetItem(const std::string& key, std::string* value);
  bool SetItem(const std::string& key, const std::string& value);
  bool RemoveItem(const std::string& key);
  void Clear();
 private:
  Widget();
  virtual ~Widget();
};
}  // namespace wrt

#endif  // WRT_EXTENSION_WIDGET_H_
