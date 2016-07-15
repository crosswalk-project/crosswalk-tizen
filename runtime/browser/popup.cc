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


#include "runtime/browser/popup.h"

#include "common/logger.h"
#include "runtime/browser/native_window.h"
#include "runtime/browser/popup_string.h"
#include "runtime/common/constants.h"

namespace runtime {

namespace {

const char* kWRTEdjePath = "/usr/share/edje/xwalk/xwalk_tizen.edj";

#ifdef MODEL_FORMFACTOR_CIRCLE
const char* kWRTIconDeletePath = "/usr/share/icons/xwalk/tw_ic_popup_btn_delete.png";
const char* kWRTIconCheckPath = "/usr/share/icons/xwalk/tw_ic_popup_btn_check.png";

const char* kLayoutTheme = "content/circle/buttons2";
const char* kContentTitle = "elm.text.title";
const char* kContentText = "elm.text";

const char* kStylePopup = "circle";
const char* kStyleCheck = "small";
const char* kStyleButtonLeft = "popup/circle/left";
const char* kStyleButtonRight = "popup/circle/right";
#else
const char* kLayoutTheme = "default";
const char* kContentTitle = "title,text";
const char* kContentText = NULL;

const char* kStylePopup = "default";
const char* kStyleCheck = "default";
const char* kStyleButtonLeft = "popup";
const char* kStyleButtonRight = "popup";
#endif  // MODEL_FORMFACTOR_CIRCLE

const char* kContentButton1 = "button1";
const char* kContentButton2 = "button2";

const char* kStyleLabel = "popup/default";
const char* kStyleButton = "popup";
const char* kStyleEditPw = "editfield/password/popup";

const char* kSignalEdit = "elm,action,hide,search_icon";

const char* kStateActivated = "activated";
const char* kStateClicked = "clicked";

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
static Evas_Object* AddButton(Popup* popup, const char* str_id,
                                  const char* content,
                                  const char* style_button) {
  Evas_Object* btn = elm_button_add(popup->popup());
  elm_object_style_set(btn, style_button);
  elm_object_domain_translatable_part_text_set(btn, 0,
                                               kTextDomainRuntime,
                                               str_id);
  elm_object_part_content_set(popup->popup(), content, btn);
  evas_object_smart_callback_add(btn, kStateClicked,
                                 ButtonClickedCallback, popup);
#ifdef MODEL_FORMFACTOR_CIRCLE
  Evas_Object* icon = elm_image_add(btn);
  if (!strcmp(content, kContentButton1)) {
    elm_image_file_set(icon, kWRTIconDeletePath, NULL);
  } else {
    elm_image_file_set(icon, kWRTIconCheckPath, NULL);
  }
  evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_object_part_content_set(btn, "elm.swallow.content", icon);
  evas_object_show(icon);
#endif  // MODEL_FORMFACTOR_CIRCLE
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
  Evas_Object* entry = AddEntry(parent, type);
  evas_object_show(entry);

  Evas_Object* layout = elm_layout_add(parent);
  elm_layout_file_set(layout, kWRTEdjePath, "PopupTextEntrySet");

  Evas_Object* rectangle = evas_object_rectangle_add(
                             evas_object_evas_get(layout));
  evas_object_color_set(rectangle, 0, 0, 0, 0);
  evas_object_resize(rectangle, 100, 100);
  evas_object_size_hint_min_set(rectangle, 100, 100);
  evas_object_show(rectangle);
  elm_object_part_content_set(layout, "entry.rectangle", rectangle);
  evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_object_domain_translatable_part_text_set(layout, "entry.text",
                                               kTextDomainRuntime,
                                               str_id);
  elm_layout_content_set(layout, "entry.swallow", entry);

  evas_object_show(layout);
  elm_box_pack_end(parent, layout);

  return entry;
}

static Evas_Object* AddCheckBox(Evas_Object* parent) {
  Evas_Object* check = elm_check_add(parent);
  elm_object_style_set(check, kStyleCheck);
  evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
#ifndef MODEL_FORMFACTOR_CIRCLE
  elm_object_style_set(check, "multiline");
#endif  // MODEL_FORMFACTOR_CIRCLE
  elm_check_state_set(check, EINA_TRUE);
  return check;
}

}  // namespace

// static variable initialize
std::set<Popup*> Popup::opened_popups_;

Popup* Popup::CreatePopup(NativeWindow* window) {
  Evas_Object* popup = elm_popup_add(window->evas_object());
  evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_object_style_set(popup, kStylePopup);

  Evas_Object* layout = elm_layout_add(popup);
  elm_layout_theme_set(layout, "layout", "popup", kLayoutTheme);
#ifdef MODEL_FORMFACTOR_CIRCLE
  elm_object_content_set(popup, layout);

  Evas_Object* box = elm_box_add(layout);
  elm_box_padding_set(box, 0, 10);
  evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_object_part_content_set(layout, "elm.swallow.content", box);
  evas_object_show(box);
#else
  //elm_object_content_set(popup, layout);

  Evas_Object* box = elm_box_add(popup);
  elm_box_padding_set(box, 0, 10);
  evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_object_part_content_set(popup, "default", box);
  evas_object_show(box);
#endif  // MODEL_FORMFACTOR_CIRCLE

  evas_object_event_callback_add(popup, EVAS_CALLBACK_RESIZE, NULL, NULL);

  return new Popup(popup, layout, box);

}

void Popup::ForceCloseAllPopup() {
  auto backup = opened_popups_;
  for (auto& popup : backup) {
    // will cause modification of opened_popups_
    popup->Hide();
  }
}

void Popup::SetButtonType(ButtonType type) {
  enable_button_ = true;
  switch (type) {
    case ButtonType::OkButton:
      button1_ = AddButton(this, popup_string::kPopupButtonOk,
                           kContentButton1, kStyleButton);
      break;
    case ButtonType::OkCancelButton:
      button1_ = AddButton(this, popup_string::kPopupButtonCancel,
                           kContentButton1, kStyleButtonLeft);
      button2_ = AddButton(this, popup_string::kPopupButtonOk,
                           kContentButton2, kStyleButtonRight);
      break;
    case ButtonType::LoginCancelButton:
      button1_ = AddButton(this, popup_string::kPopupButtonCancel,
                           kContentButton1, kStyleButtonLeft);
      button2_ = AddButton(this, popup_string::kPopupButtonLogin,
                           kContentButton2, kStyleButtonRight);
      break;
    case ButtonType::AllowDenyButton:
      button1_ = AddButton(this, popup_string::kPopupButtonDeny,
                           kContentButton1, kStyleButtonLeft);
      button2_ = AddButton(this, popup_string::kPopupButtonAllow,
                           kContentButton2, kStyleButtonRight);
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
    elm_object_domain_translatable_part_text_set(
        check_box_, kContentText,
        kTextDomainRuntime,
        str_id.c_str());
  }
#ifdef MODEL_FORMFACTOR_CIRCLE
  elm_object_part_content_set(box_, "elm.swallow.checkbox", check_box_);
  evas_object_size_hint_min_set(check_box_, 0, 80);
#endif  // MODEL_FORMFACTOR_CIRCLE
  elm_box_pack_end(box_, check_box_);
  evas_object_show(check_box_);
}

bool Popup::GetCheckBoxResult() const {
  return result_check_box_;
}

void Popup::SetTitle(const std::string& str_id) {
  elm_object_domain_translatable_part_text_set(
#ifdef MODEL_FORMFACTOR_CIRCLE
      layout_,
#else
      popup_,
#endif  // MODEL_FORMFACTOR_CIRCLE
      kContentTitle,
      kTextDomainRuntime,
      str_id.c_str());
}

void Popup::SetBody(const std::string& str_id) {
  Evas_Object* label = elm_label_add(box_);
  elm_object_style_set(label, kStyleLabel);
  elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
  elm_object_domain_translatable_part_text_set(
                     label, kContentText, kTextDomainRuntime, str_id.c_str());
  evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
#ifdef MODEL_FORMFACTOR_CIRCLE
  evas_object_color_set(label, 255, 255, 255, 255);
  elm_object_part_content_set(box_, "elm.swallow.label", label);
#else
  evas_object_color_set(label, 0, 0, 0, 255);
#endif  // MODEL_FORMFACTOR_CIRCLE
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
  opened_popups_.insert(this);
}

void Popup::Hide() {
  evas_object_hide(popup_);
  ecore_idler_add([](void* popup) {
      Popup* obj = static_cast<Popup*>(popup);
      delete obj;
      return EINA_FALSE;
    }, this);
  auto found = opened_popups_.find(this);
  if (found != opened_popups_.end()) {
    opened_popups_.erase(found);
  }
}

void Popup::Result(bool is_positive) {
  if (enable_button_) {
    result_button_ = is_positive;
  }
  if (enable_entry_ && !!entry1_) {
    const char* text = elm_entry_entry_get(entry1_);
    if (text)
      result_entry1_ = text;
    if (!!entry2_) {
      text = elm_entry_entry_get(entry2_);
      if (text)
        result_entry2_ = text;
    }
  }
  if (enable_check_box_) {
    result_check_box_ = elm_check_state_get(check_box_);
  }

  handler_(this, user_data_);
}

Popup::Popup(Evas_Object* popup, Evas_Object* layout, Evas_Object* box)
  : popup_(popup), layout_(layout), box_(box), button1_(NULL), button2_(NULL),
    entry1_(NULL), entry2_(NULL), check_box_(NULL), user_data_(NULL),
    enable_button_(false), result_button_(false), enable_entry_(false),
    enable_check_box_(false), result_check_box_(false) {}

Popup::~Popup() {
  if (popup_)
    evas_object_del(popup_);
  popup_ = NULL;
}

}  // namespace runtime
