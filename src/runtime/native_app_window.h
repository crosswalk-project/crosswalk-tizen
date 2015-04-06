// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_NATIVE_APP_WINDOW_H_
#define WRT_RUNTIME_NATIVE_APP_WINDOW_H_

#include "runtime/native_window.h"

namespace wrt {

class NativeAppWindow: public NativeWindow {
 public:
  NativeAppWindow();
  virtual ~NativeAppWindow();
 protected:
  Evas_Object* createWindowInternal();  // override
};

}  // namespace wrt

#endif  // WRT_RUNTIME_NATIVE_APP_WINDOW_H_
