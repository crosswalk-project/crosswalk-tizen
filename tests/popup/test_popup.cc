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


#include <Elementary.h>
#include <string>
#include <iostream>
#include "popup/popup.h"
#include "popup/popup_string.h"

int main(int argc, char **argv) {
  const char* kLocaleKorean = "ko_KR";
  setlocale(LC_ALL, kLocaleKorean);
  bindtextdomain("wrt", "/usr/share/locale");

  Evas_Object* win;

  elm_init(argc, argv);
  elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

  win = elm_win_util_standard_add("sample", "Sample");
  elm_win_autodel_set(win, EINA_TRUE);

  const std::string title = wrt::popup_string::kPopupTitleCert;
  const std::string body_text =
    wrt::popup_string::GetText(wrt::popup_string::kPopupBodyCert) +
    "<br>" + "http://url.url";
  const std::string check_label =
    wrt::popup_string::kPopupCheckRememberPreference;
  const std::string entry_1_label = wrt::popup_string::kPopupLabelAuthusername;
  const std::string entry_2_label = wrt::popup_string::kPopupLabelPassword;

  wrt::Popup* popup = wrt::Popup::CreatePopup(win);
  popup->SetTitle(title);
  popup->SetBody(body_text);
  popup->SetFirstEntry(entry_1_label, wrt::Popup::EntryType::Edit);
  popup->SetSecondEntry(entry_2_label, wrt::Popup::EntryType::PwEdit);
  popup->SetCheckBox(check_label);
  popup->SetButtonType(wrt::Popup::ButtonType::AllowDenyButton);
  popup-> SetResultHandler([](wrt::Popup* popup, void* user_data) {
    std::cout<< popup->GetFirstEntryResult() << std::endl;
    std::cout<< popup->GetSecondEntryResult() << std::endl;
    std::cout<< popup->GetCheckBoxResult() << std::endl;
    std::cout<< popup->GetButtonResult() << std::endl;
  }, NULL);
  popup->Show();

  elm_language_set(kLocaleKorean);

  evas_object_resize(win, 200, 100);
  evas_object_show(win);

  elm_run();
  elm_shutdown();

  return 0;
}
