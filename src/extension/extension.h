// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_EXTENSION_EXTENSION_H_
#define WRT_EXTENSION_EXTENSION_H_

#include <string>
#include <vector>

namespace wrt {

class Extension {
 public:
  Extension();
  virtual ~Extension();

  std::string name() const { return name_; }
  std::string javascript_api() const { return javascript_api_; }
  const std::vector<std::string>& entry_points() { return entry_points_; }

 protected:
  std::string name_;
  std::string javascript_api_;
  std::vector<std::string> entry_points_;

 private:
};

}  // namespace wrt

#endif  // WRT_EXTENSION_EXTENSION_H_
