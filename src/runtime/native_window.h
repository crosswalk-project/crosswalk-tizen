// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_NATIVE_WINDOW_H_
#define WRT_RUNTIME_NATIVE_WINDOW_H_
#include <Elementary.h>

namespace wrt {

class NativeWindow {
 public:
  NativeWindow();
  virtual ~NativeWindow();

  void Initialize();

  bool initialized() const { return initialized_; }
  Evas_Object* evas_object() const;
  void SetContent(Evas_Object* content);

 protected:
  virtual Evas_Object* createWindowInternal() = 0;


 private:
  static void didDeleteRequested(void* data, Evas_Object* obj, void* event_info);
  static void didProfileChanged(void* data, Evas_Object* obj, void* event_info);



  bool initialized_;
  Evas_Object* window_;


};

} // namespace wrt


#endif // WRT_RUNTIME_NATIVE_WINDOW_H_
