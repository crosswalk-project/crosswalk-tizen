// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "web_application.h"

namespace wrt {

WebApplication::WebApplication(const std::string& appid)
    : initialized_(false),
      appid_(appid) {
}

WebApplication::~WebApplication() {
}

void WebApplication::Launch() {

  initialized_ = true;
}

void WebApplication::Resume() {

}

void WebApplication::Suspend() {

}


} // namespace wrt
