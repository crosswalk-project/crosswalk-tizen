// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_APP_CONTROL_H_
#define WRT_RUNTIME_APP_CONTROL_H_

#include <app_control.h>
#include <bundle.h>
#include <string>
#include <vector>

namespace wrt {

class AppControl {
 public:
  explicit AppControl(app_control_h app_control);
  ~AppControl();

  std::string operation() const;
  std::string mime() const;
  std::string uri() const;
  std::string category() const;
  std::string data(const std::string& key) const;
  std::vector<std::string> data_array(const std::string& key) const;
  std::string encoded_bundle() const;

  bool IsDataArray(const std::string& key);
  bool AddData(const std::string& key, const std::string& value);
  bool AddDataArray(const std::string& key,
                    const std::vector<std::string>& value_array);

 private:
  app_control_h app_control_;
  bundle* app_control_bundle_;
  bundle_raw* app_control_bundle_raw_;
  int app_control_bundle_raw_len_;
};

}  // namespace wrt

#endif  // WRT_RUNTIME_APP_CONTROL_H_
