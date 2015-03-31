// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_WEB_APPLICATION_H_
#define WRT_RUNTIME_WEB_APPLICATION_H_

#include <string>

namespace wrt {

class WebApplication {
 public:
  WebApplication(const std::string& appid);
  virtual ~WebApplication();

  void Launch();
  void Resume();
  void Suspend();

  bool initialized() const { return initialized_; }

 private:
  bool initialized_;
  std::string appid_;

};

} // namespace wrt


#endif // WRT_RUNTIME_WEB_APPLICATION_H_
