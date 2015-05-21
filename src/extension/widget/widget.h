// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
