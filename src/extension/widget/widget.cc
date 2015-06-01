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
#include "extension/widget/widget.h"

#include <list>
#include <memory>
#include <map>
#include <vector>

#include "extension/xwalk/XW_Extension.h"
#include "extension/xwalk/XW_Extension_EntryPoints.h"
#include "extension/xwalk/XW_Extension_Permissions.h"
#include "extension/xwalk/XW_Extension_Runtime.h"
#include "extension/xwalk/XW_Extension_SyncMessage.h"

#include "extension/widget/picojson.h"
#include "common/logger.h"
#include "common/app_db.h"
#include "common/application_data.h"
#include "common/locale_manager.h"
#include "common/string_utils.h"

XW_Extension g_xw_extension = 0;
std::string g_appid;
std::unique_ptr<wrt::ApplicationData> g_appdata;
const XW_CoreInterface* g_core = NULL;
const XW_MessagingInterface* g_messaging = NULL;
const XW_Internal_SyncMessagingInterface* g_sync_messaging = NULL;
const XW_Internal_EntryPointsInterface* g_entry_points = NULL;
const XW_Internal_RuntimeInterface* g_runtime = NULL;
extern const char kSource_widget_api[];

typedef void (*CmdHandler)(const picojson::value& args, picojson::object* out);

static void InitHandler(const picojson::value& args, picojson::object* out);
static void KeyHandler(const picojson::value& args, picojson::object* out);
static void GetItemHandler(const picojson::value& args,
                           picojson::object* out);
static void LengthHandler(const picojson::value& args,
                           picojson::object* out);
static void ClearHandler(const picojson::value& args,
                           picojson::object* out);
static void SetItemHandler(const picojson::value& args,
                           picojson::object* out);
static void RemoveHandler(const picojson::value& args,
                           picojson::object* out);

std::map<std::string, CmdHandler> g_handler = {
  {"init", InitHandler},
  {"key", KeyHandler},
  {"length", LengthHandler},
  {"clear", ClearHandler},
  {"getItem", GetItemHandler},
  {"setItem", SetItemHandler},
  {"removeItem", RemoveHandler},
};


static void HandleMessage(XW_Instance instance,
                          const char* message,
                          bool sync);

extern "C" int32_t XW_Initialize(XW_Extension extension,
                                 XW_GetInterface get_interface) {
  g_xw_extension = extension;
  g_core = reinterpret_cast<const XW_CoreInterface*>(
      get_interface(XW_CORE_INTERFACE));
  if (!g_core) {
    LOGGER(ERROR)
        << "Can't initialize extension: error getting Core interface.";
    return XW_ERROR;
  }

  g_messaging = reinterpret_cast<const XW_MessagingInterface*>(
      get_interface(XW_MESSAGING_INTERFACE));
  if (!g_messaging) {
    LOGGER(ERROR)
        << "Can't initialize extension: error getting Messaging interface.";
    return XW_ERROR;
  }

  g_sync_messaging =
      reinterpret_cast<const XW_Internal_SyncMessagingInterface*>(
          get_interface(XW_INTERNAL_SYNC_MESSAGING_INTERFACE));
  if (!g_sync_messaging) {
    LOGGER(ERROR)
        << "Can't initialize extension: "
        << "error getting SyncMessaging interface.";
    return XW_ERROR;
  }

  g_entry_points = reinterpret_cast<const XW_Internal_EntryPointsInterface*>(
      get_interface(XW_INTERNAL_ENTRY_POINTS_INTERFACE));
  if (!g_entry_points) {
    LOGGER(ERROR)
        << "NOTE: Entry points interface not available in this version "
        << "of Crosswalk, ignoring entry point data for extensions.\n";
    return XW_ERROR;
  }

  g_runtime = reinterpret_cast<const XW_Internal_RuntimeInterface*>(
      get_interface(XW_INTERNAL_RUNTIME_INTERFACE));
  if (!g_runtime) {
    LOGGER(ERROR)
        << "NOTE: runtime interface not available in this version "
        << "of Crosswalk, ignoring runtime variables for extensions.\n";
    return XW_ERROR;
  }

  std::vector<char> res(256, 0);
  g_runtime->GetRuntimeVariableString(extension, "app_id", &res[0], 256);
  g_appid = std::string(res.begin(), res.end());
  if (g_appid.at(0) == '"') {
    g_appid = g_appid.substr(1, strlen(g_appid.c_str())-2);
  }

  g_core->RegisterInstanceCallbacks(
      g_xw_extension,
      [](XW_Instance /*instance*/){
        if (g_appdata.get() == NULL) {
          g_appdata.reset(new wrt::ApplicationData(g_appid));
          g_appdata->LoadManifestData();
        }
        wrt::Widget::GetInstance()->Initialize(g_appdata.get());
      },
      NULL);

  g_messaging->Register(g_xw_extension, [](XW_Instance instance,
                                           const char* message) {
    HandleMessage(instance, message, false);
  });
  g_sync_messaging->Register(g_xw_extension, [](XW_Instance instance,
                                                const char* message) {
    HandleMessage(instance, message, true);
  });

  g_core->SetExtensionName(g_xw_extension, "Widget");
  const char* entry_points[] = {"widget", NULL};
  g_entry_points->SetExtraJSEntryPoints(g_xw_extension, entry_points);
  g_core->SetJavaScriptAPI(g_xw_extension, kSource_widget_api);

  return XW_OK;
}

static void InitHandler(const picojson::value& /*args*/,
                        picojson::object* out) {
  picojson::value result = picojson::value(picojson::object());
  picojson::object& obj = result.get<picojson::object>();

  auto widget_info = g_appdata->widget_info();
  if (widget_info.get() == NULL) {
    out->insert(std::make_pair("status", picojson::value("error")));
    return;
  }
  out->insert(std::make_pair("status", picojson::value("success")));

  wrt::LocaleManager locale_manager;
  if (!widget_info->default_locale().empty()) {
    locale_manager.SetDefaultLocale(widget_info->default_locale());
  }

  // TODO(sngn.lee): should be returned localized string
  obj["author"] = picojson::value(widget_info->author());
  obj["description"] = picojson::value(
      locale_manager.GetLocalizedString(widget_info->description_set()));
  obj["name"] = picojson::value(
      locale_manager.GetLocalizedString(widget_info->name_set()));
  obj["shortName"] = picojson::value(
      locale_manager.GetLocalizedString(widget_info->short_name_set()));
  obj["version"] = picojson::value(widget_info->version());
  obj["id"] = picojson::value(widget_info->id());
  obj["authorEmail"] = picojson::value(widget_info->author_email());
  obj["authorHref"] = picojson::value(widget_info->author_href());
  obj["height"] = picojson::value(static_cast<double>(widget_info->height()));
  obj["width"] = picojson::value(static_cast<double>(widget_info->width()));

  out->insert(std::make_pair("result", result));
}

static void KeyHandler(const picojson::value& args, picojson::object* out) {
  int idx = static_cast<int>(args.get("idx").get<double>());
  std::string key;
  if (!wrt::Widget::GetInstance()->Key(idx, &key)) {
    out->insert(std::make_pair("status", picojson::value("error")));
    return;
  }
  out->insert(std::make_pair("status", picojson::value("success")));
  out->insert(std::make_pair("result", picojson::value(key)));
}

static void GetItemHandler(const picojson::value& args,
                           picojson::object* out) {
  const std::string& key = args.get("key").get<std::string>();
  std::string value;
  if (!wrt::Widget::GetInstance()->GetItem(key, &value)) {
    out->insert(std::make_pair("status", picojson::value("error")));
    return;
  }
  out->insert(std::make_pair("status", picojson::value("success")));
  out->insert(std::make_pair("result", picojson::value(value)));
}

static void LengthHandler(const picojson::value& /*args*/,
                           picojson::object* out) {
  int length = wrt::Widget::GetInstance()->Length();
  out->insert(std::make_pair("status", picojson::value("success")));
  out->insert(
      std::make_pair("result", picojson::value(static_cast<double>(length))));
}

static void ClearHandler(const picojson::value& /*args*/,
                           picojson::object* out) {
  wrt::Widget::GetInstance()->Clear();
  out->insert(std::make_pair("status", picojson::value("success")));
}

static void SetItemHandler(const picojson::value& args,
                           picojson::object* out) {
  const std::string& key = args.get("key").get<std::string>();
  const std::string& value = args.get("value").get<std::string>();
  std::string oldvalue;
  if (wrt::Widget::GetInstance()->GetItem(key, &oldvalue)) {
    out->insert(std::make_pair("result", picojson::value(oldvalue)));
  } else {
    out->insert(std::make_pair("result", picojson::value()));
  }
  wrt::Widget::GetInstance()->SetItem(key, value);
  out->insert(std::make_pair("status", picojson::value("success")));
}

static void RemoveHandler(const picojson::value& args,
                           picojson::object* out) {
  const std::string& key = args.get("key").get<std::string>();
  std::string oldvalue;
  if (wrt::Widget::GetInstance()->GetItem(key, &oldvalue)) {
    out->insert(std::make_pair("result", picojson::value(oldvalue)));
  } else {
    out->insert(std::make_pair("result", picojson::value()));
  }
  wrt::Widget::GetInstance()->RemoveItem(key);
  out->insert(std::make_pair("status", picojson::value("success")));
}


static void HandleMessage(XW_Instance instance,
                          const char* message,
                          bool sync) {
  picojson::value value;
  std::string err;
  picojson::parse(value, message, message + strlen(message), &err);
  if (!err.empty()) {
    LOGGER(ERROR) << "Ignoring message. " << err;
    return;
  }

  if (!value.is<picojson::object>()) {
    LOGGER(ERROR) << "Ignoring message. It is not an object.";
    return;
  }

  std::string cmd = value.get("cmd").to_str();
  // check for args in JSON message
  const picojson::value& args = value.get("args");
  picojson::value result = picojson::value(picojson::object());

  auto handler = g_handler.find(cmd);
  if (handler != g_handler.end()) {
    handler->second(args, &result.get<picojson::object>());
  } else {
    result.get<picojson::object>().insert(
        std::make_pair("status", picojson::value("error")));
  }

  if (sync) {
    g_sync_messaging->SetSyncReply(instance, result.serialize().c_str());
  }
}

namespace wrt {

namespace {
  const char* kDbInitedCheckKey = "__WRT_DB_INITED__";
  const char* kDBPublicSection = "public";
  const char* kDBPrivateSection = "private";
}  // namespace


Widget* Widget::GetInstance() {
  static Widget instance;
  return &instance;
}

Widget::Widget() {
}
Widget::~Widget() {
}

void Widget::Initialize(const ApplicationData* appdata) {
  AppDB* db = AppDB::GetInstance();
  if (db->HasKey(kDBPrivateSection, kDbInitedCheckKey))
    return;
  if (appdata->widget_info() == NULL)
    return;

  auto preferences = appdata->widget_info()->preferences();
  auto it = preferences.begin();
  for ( ; it != preferences.end(); ++it) {
    db->Set(kDBPublicSection,
            (*it)->Name(),
            (*it)->Value());
  }
  db->Set(kDBPrivateSection, kDbInitedCheckKey, "true");
}

int Widget::Length() {
  AppDB* db = AppDB::GetInstance();
  std::list<std::string> list;
  db->GetKeys(kDBPublicSection, &list);
  return list.size();
}

bool Widget::Key(int idx, std::string* key) {
  AppDB* db = AppDB::GetInstance();
  std::list<std::string> list;
  db->GetKeys(kDBPublicSection, &list);

  auto it = list.begin();
  for ( ; it != list.end() && idx >= 0; ++it) {
    if (idx == 0) {
      *key = *it;
      return true;
    }
    idx--;
  }
  return false;
}

bool Widget::GetItem(const std::string& key, std::string* value) {
  AppDB* db = AppDB::GetInstance();
  if (!db->HasKey(kDBPublicSection, key))
    return false;
  *value = db->Get(kDBPublicSection, key);
  return true;
}

bool Widget::SetItem(const std::string& key, const std::string& value) {
  AppDB* db = AppDB::GetInstance();
  db->Set(kDBPublicSection, key, value);
  return true;
}

bool Widget::RemoveItem(const std::string& key) {
  AppDB* db = AppDB::GetInstance();
  if (!db->HasKey(kDBPublicSection, key))
    return false;
  db->Remove(kDBPublicSection, key);
  return true;
}

void Widget::Clear() {
  AppDB* db = AppDB::GetInstance();
  std::list<std::string> list;
  db->GetKeys(kDBPublicSection, &list);
  auto it = list.begin();
  for ( ; it != list.end(); ++it) {
    db->Remove(kDBPublicSection, *it);
  }
}

}  // namespace wrt
