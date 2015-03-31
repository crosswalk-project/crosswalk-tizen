// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_RUNTIME_H_
#define WRT_RUNTIME_RUNTIME_H_

#include <app.h>

#include "web_application.h"
#include "native_window.h"

namespace wrt {

class Runtime {
 public:
  Runtime();
  virtual ~Runtime();

  int Exec(int argc, char* argv[]);
 private:
  static bool onCreate(void* data);
  static void onTerminate(void* data);
  static void onPause(void* data);
  static void onResume(void* data);
  static void onAppControl(app_control_h app_control, void* data);

  void createNativeWindow();

  WebApplication* application_;
  NativeWindow* native_window_;
};

} // namespace wrt

#endif // WRT_RUNTIME_RUNTIME_H_
