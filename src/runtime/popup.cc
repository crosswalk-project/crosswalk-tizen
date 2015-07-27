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


#include "runtime/popup.h"

#include "runtime/native_window.h"
#include "runtime/popup_string.h"
#include "common/logger.h"

namespace wrt {

namespace {

const char* kContentTitle = "title,text";
const char* kContentButton1 = "button1";
const char* kContentButton2 = "button2";

const char* kStyleDefault = "default";
const char* kStyleLabel = "default";
const char* kStyleButton = "popup";
const char* kStyleEditPw = "editfield/password/popup";

const char* kSignalEdit = "elm,action,hide,search_icon";

const char* kStateActivated = "activated";
const char* kStateClicked = "clicked";

const double kMaxPopupHeight = 0.80;
const double kMaxScrollerHeight = 0.80;

static void ButtonClickedCallback(void* data,
                                  Evas_Object* obj, void* /*eventInfo*/) {
  Popup* popup = static_cast<Popup*>(data);
  if (!popup) {
    LOGGER(ERROR) << "Fail to get Popup instance";
    return;
  }
  popup->Result(popup->IsPositiveButton(obj));
  popup->Hide();
}

// caution: not Evas_Object* but Popup*
static Evas_Object* AddButton(Popup* popup,
                              const char* str_id, const char* content) {
  Evas_Object* btn = elm_button_add(popup->popup());
  elm_object_style_set(btn, kStyleButton);
  elm_object_domain_translatable_part_text_set(btn, 0,
                                               popup_string::kTextDomainWrt,
                                               str_id);
  elm_object_part_content_set(popup->popup(), content, btn);
  evas_object_smart_callback_add(btn, kStateClicked,
                                 ButtonClickedCallback, popup);
  return btn;
}

static Evas_Object* AddEntry(Evas_Object* parent, Popup::EntryType type) {
  Evas_Object* entry = elm_entry_add(parent);
  elm_object_style_set(entry, kStyleEditPw);
  elm_entry_single_line_set(entry, EINA_TRUE);
  elm_entry_scrollable_set(entry, EINA_TRUE);
  evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_entry_prediction_allow_set(entry, EINA_FALSE);
  elm_object_signal_emit(entry, kSignalEdit, "");
  elm_entry_autocapital_type_set(entry, ELM_AUTOCAPITAL_TYPE_NONE);

  if (type == Popup::EntryType::Edit) {
    evas_object_smart_callback_add(entry, kStateActivated,
                                   [](void*, Evas_Object* obj, void*) {
                                     elm_object_focus_set(obj, EINA_TRUE);
                                   }, NULL);
  } else {
    elm_entry_password_set(entry, EINA_TRUE);
    elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_PASSWORD);
  }

  return entry;
}

static Evas_Object* AddEntrySet(Evas_Object* parent,
                                const char* str_id, Popup::EntryType type) {
  // a grid for entry
  Evas_Object* entry_grid = elm_grid_add(parent);
  evas_object_size_hint_weight_set(entry_grid, EVAS_HINT_EXPAND,
                                   EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(entry_grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
  evas_object_show(entry_grid);

  // label for the entry
  Evas_Object* entry_label = elm_label_add(entry_grid);
  elm_object_style_set(entry_label, kStyleLabel);
  elm_object_domain_translatable_part_text_set(entry_label, 0,
                                               popup_string::kTextDomainWrt,
                                               str_id);
  evas_object_color_set(entry_label, 0, 0, 0, 255);
  evas_object_size_hint_weight_set(entry_label,
                                   EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(entry_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_grid_pack(entry_grid, entry_label, 0, 0, 30, 100);
  evas_object_show(entry_label);

  // entry
  Evas_Object* entry = AddEntry(entry_grid, type);
  evas_object_show(entry);
  elm_grid_pack(entry_grid, entry, 30, 0, 40, 100);
  elm_box_pack_end(parent, entry_grid);

  return entry;
}

static Evas_Object* AddCheckBox(Evas_Object* parent) {
  Evas_Object* check = elm_check_add(parent);
  elm_object_style_set(check, kStyleDefault);
  elm_object_style_set(check, "multiline");
  evas_object_size_hint_align_set(check, 0.0, 0.0);
  evas_object_color_set(check, 0, 0, 0, 255);
  elm_check_state_set(check, EINA_TRUE);
  return check;
}

}  // namespace

Popup* Popup::CreatePopup(NativeWindow* window) {
  Evas_Object* popup = elm_popup_add(window->evas_object());
  elm_object_style_set(popup, kStyleDefault);

  Evas_Object* grid = elm_grid_add(popup);
  evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_object_part_content_set(popup, "default", grid);
  evas_object_show(grid);

  Evas_Object* box = elm_box_add(grid);
  elm_box_padding_set(box, 0, 10);
  evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_grid_pack(grid, box, 3, 3, 94, 94);
  evas_object_show(box);

  evas_object_event_callback_add(popup, EVAS_CALLBACK_RESIZE, NULL, NULL);

  return new Popup(popup, grid, box);
}

void Popup::SetButtonType(ButtonType type) {
  enable_button_ = true;
  switch (type) {
    case ButtonType::OkButton:
    button1_ = AddButton(this, popup_string::kPopupButtonOk,
                         kContentButton1);
    break;
    case ButtonType::OkCancelButton:
    button1_ = AddButton(this, popup_string::kPopupButtonCancel,
                         kContentButton1);
    button2_ = AddButton(this, popup_string::kPopupButtonOk,
                         kContentButton2);
    case ButtonType::LoginCancelButton:
    button1_ = AddButton(this, popup_string::kPopupButtonCancel,
                         kContentButton1);
    button2_ = AddButton(this, popup_string::kPopupButtonLogin,
                         kContentButton2);
    break;
    case ButtonType::AllowDenyButton:
    button1_ = AddButton(this, popup_string::kPopupButtonDeny,
                         kContentButton1);
    button2_ = AddButton(this, popup_string::kPopupButtonAllow,
                         kContentButton2);
    break;
  }
}

bool Popup::IsPositiveButton(Evas_Object* button) {
  if (button == NULL || button1_ == NULL)
    return false;
  else
    return button == button2_;
}

bool Popup::GetButtonResult() const {
  return result_button_;
}

void Popup::SetFirstEntry(const std::string& str_id, EntryType type) {
  enable_entry_ = true;
  entry1_ = AddEntrySet(box_, str_id.c_str(), type);
}

// suppose that it is called after SetFirstEntry()
void Popup::SetSecondEntry(const std::string& str_id, EntryType type) {
  if (!enable_entry_ || !entry1_) {
    LOGGER(ERROR) << "SetFirstEntry() is not called yet";
    return;
  }
  entry2_ = AddEntrySet(box_, str_id.c_str(), type);
}

std::string Popup::GetFirstEntryResult() const {
  return result_entry1_;
}

std::string Popup::GetSecondEntryResult() const {
  return result_entry2_;
}

void Popup::SetCheckBox(const std::string& str_id) {
  enable_check_box_ = true;
  check_box_ = AddCheckBox(box_);
  if (!str_id.empty()) {
    elm_object_domain_translatable_part_text_set(check_box_, 0,
                                                 popup_string::kTextDomainWrt,
                                                 str_id.c_str());
  }
  elm_box_pack_end(box_, check_box_);
  evas_object_show(check_box_);
}

bool Popup::GetCheckBoxResult() const {
  return result_check_box_;
}

void Popup::SetTitle(const std::string& str_id) {
  elm_object_domain_translatable_part_text_set(popup_, kContentTitle,
                                               popup_string::kTextDomainWrt,
                                               str_id.c_str());
}

void Popup::SetBody(const std::string& str_id) {
  Evas_Object* label = elm_label_add(box_);
  elm_object_style_set(label, kStyleLabel);
  elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
  elm_object_domain_translatable_part_text_set(label, 0,
                                               popup_string::kTextDomainWrt,
                                               str_id.c_str());
  evas_object_color_set(label, 0, 0, 0, 255);
  evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_box_pack_end(box_, label);
  evas_object_show(label);
}

void Popup::SetResultHandler(std::function<void
    (Popup* popup, void* user_data)> handler, void* user_data) {
  handler_ = handler;
  user_data_ = user_data;
}

void Popup::Show() {
  evas_object_show(popup_);
}

void Popup::Hide() {
  evas_object_hide(popup_);
  ecore_idler_add([](void* popup) {
      Popup* obj = static_cast<Popup*>(popup);
      delete obj;
      return EINA_FALSE;
    }, this);
}

void Popup::Result(bool is_positive) {
  if (enable_button_) {
    result_button_ = is_positive;
  }
  if (enable_entry_ && !!entry1_) {
    result_entry1_ = elm_entry_entry_get(entry1_);
    if (!!entry2_) {
      result_entry2_ = elm_entry_entry_get(entry2_);
    }
  }
  if (enable_check_box_) {
    result_check_box_ = elm_check_state_get(check_box_);
  }

  handler_(this, user_data_);
}

Popup::Popup(Evas_Object* popup, Evas_Object* grid, Evas_Object* box)
  : popup_(popup), grid_(grid), box_(box) {}

Popup::~Popup() {
  if (popup_)
    evas_object_del(popup_);
  popup_ = NULL;
}

}  // namespace wrt
