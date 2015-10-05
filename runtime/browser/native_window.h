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

#ifndef XWALK_RUNTIME_BROWSER_NATIVE_WINDOW_H_
#define XWALK_RUNTIME_BROWSER_NATIVE_WINDOW_H_

#include <Elementary.h>

#include <functional>
#include <map>

namespace runtime {

class NativeWindow {
 public:
  enum class ScreenOrientation {
    PORTRAIT_PRIMARY = 0,
    PORTRAIT_SECONDARY = 1,
    LANDSCAPE_PRIMARY = 2,
    LANDSCAPE_SECONDARY = 3,
    NATURAL = 4,
    ANY = 5
  };
  typedef std::function<void(int)> RotationHandler;
  NativeWindow();
  virtual ~NativeWindow();

  void Initialize();

  bool initialized() const { return initialized_; }
  Evas_Object* evas_object() const;
  void SetContent(Evas_Object* content);
  void SetRotationLock(int degree);
  void SetRotationLock(ScreenOrientation orientation);
  void SetAutoRotation();
  int AddRotationHandler(RotationHandler handler);
  void RemoveRotationHandler(int id);
  int rotation() const { return rotation_; }
  void Show();
  void Active();
  void InActive();
  void FullScreen(bool enable);
  ScreenOrientation natural_orientation() const { return natural_orientation_;}

 protected:
  virtual Evas_Object* CreateWindowInternal() = 0;

 private:
  static void DidDeleteRequested(void* data, Evas_Object* obj,
                                 void* event_info);
  static void DidProfileChanged(void* data, Evas_Object* obj, void* event_info);
  void DidRotation(int degree);
  void DidFocusChanged(bool got);


  bool initialized_;
  Evas_Object* window_;
  Evas_Object* focus_;
  Evas_Object* content_;
  int rotation_;
  int handler_id_;
  ScreenOrientation natural_orientation_;
  std::map<int, RotationHandler> handler_table_;
};

}  // namespace runtime


#endif  // XWALK_RUNTIME_BROWSER_NATIVE_WINDOW_H_
