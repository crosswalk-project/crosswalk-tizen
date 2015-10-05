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


#ifndef XWALK_RUNTIME_BROWSER_POPUP_H_
#define XWALK_RUNTIME_BROWSER_POPUP_H_

#include <Elementary.h>
#include <Evas.h>

#include <functional>
#include <set>
#include <string>
#include <vector>

namespace runtime {

class NativeWindow;

class Popup {
 public:
  enum class ButtonType {
    OkButton,
    OkCancelButton,
    LoginCancelButton,
    AllowDenyButton
  };

  enum class EntryType {
    Edit,
    PwEdit
  };

  static Popup* CreatePopup(NativeWindow* window);
  static void ForceCloseAllPopup();

  // button
  void SetButtonType(ButtonType type);
  bool IsPositiveButton(Evas_Object* button);
  bool GetButtonResult() const;  // yes/allow/ok: true, the others: false

  void SetFirstEntry(const std::string& str_id, EntryType type);
  void SetSecondEntry(const std::string& str_id, EntryType type);
  std::string GetFirstEntryResult() const;
  std::string GetSecondEntryResult() const;

  // check box
  void SetCheckBox(const std::string& str_id = std::string());
  bool GetCheckBoxResult() const;

  // etc.
  void SetTitle(const std::string& str_id);
  void SetBody(const std::string& str_id);
  void SetResultHandler(std::function
      <void(Popup* popup, void* user_data)> handler, void* user_data);

  // Popup's actions
  void Show();
  void Hide();
  void Result(bool is_positive);

  // getter
  Evas_Object* popup() { return popup_; }

 private:
  Popup(Evas_Object* popup, Evas_Object* box);
  ~Popup();

  Evas_Object* popup_;
  Evas_Object* box_;
  Evas_Object* button1_;
  Evas_Object* button2_;
  Evas_Object* entry1_;
  Evas_Object* entry2_;
  Evas_Object* check_box_;

  std::function<void(Popup* popup, void* user_data)> handler_;
  void* user_data_;

  bool enable_button_;
  bool result_button_;
  bool enable_entry_;
  std::string result_entry1_;
  std::string result_entry2_;
  bool enable_check_box_;
  bool result_check_box_;
  static std::set<Popup*> opened_popups_;
};

}  // namespace runtime

#endif  // XWALK_RUNTIME_BROWSER_POPUP_H_
