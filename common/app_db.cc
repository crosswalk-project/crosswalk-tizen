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

#include "common/app_db.h"

//  #define USE_APP_PREFERENCE;
#ifdef USE_APP_PREFERENCE
#include <app_preference.h>
#else
#include <app.h>
#include <sqlite3.h>
#include <unistd.h>
#endif

#include <memory>

#include "common/logger.h"
#include "common/string_utils.h"
#ifndef USE_APP_PREFERENCE
#include "common/app_db_sqlite.h"
#endif

namespace common {

namespace {
#ifdef USE_APP_PREFERENCE
const char* kSectionPrefix = "_SECT_";
const char* kSectionSuffix = "_SECT_";
#else
const char* kCreateDbQuery = "CREATE TABLE IF NOT EXISTS appdb ("
                             "section TEXT, "
                             "key TEXT, "
                             "value TEXT,"
                             "PRIMARY KEY(section, key));";
#endif
}  // namespace

#ifdef USE_APP_PREFERENCE

class PreferenceAppDB : public AppDB {
 public:
  PreferenceAppDB();
  virtual bool HasKey(const std::string& section,
                      const std::string& key) const;
  virtual std::string Get(const std::string& section,
                          const std::string& key) const;
  virtual void Set(const std::string& section,
                   const std::string& key,
                   const std::string& value);
  virtual void GetKeys(const std::string& section,
                       std::list<std::string>* keys) const;
  virtual void Remove(const std::string& section,
                      const std::string& key);
};

PreferenceAppDB::PreferenceAppDB() {
}

bool PreferenceAppDB::HasKey(const std::string& section,
                             const std::string& key) const {
  bool existed = false;
  std::string combined_key = kSectionPrefix + section + kSectionSuffix + key;
  return preference_is_existing(combined_key.c_str(), &existed) == 0 && existed;
}

std::string PreferenceAppDB::Get(const std::string& section,
                                 const std::string& key) const {
  std::string combined_key = kSectionPrefix + section + kSectionSuffix + key;
  char* value;
  if (preference_get_string(combined_key.c_str(), &value) == 0) {
    std::unique_ptr<char, decltype(std::free)*> ptr {value, std::free};
    return std::string(value);
  }
  return std::string();
}

void PreferenceAppDB::Set(const std::string& section,
                          const std::string& key,
                          const std::string& value) {
  std::string combined_key = kSectionPrefix + section + kSectionSuffix + key;
  preference_set_string(combined_key.c_str(), value.c_str());
}

void PreferenceAppDB::GetKeys(const std::string& section,
                              std::list<std::string>* keys) const {
  auto callback = [](const char* key, void *user_data) {
    auto list = static_cast<std::list<std::string>*>(user_data);
    if (utils::StartsWith(key, list->front())) {
      list->push_back(key+list->front().size());
    }
    return true;
  };
  std::string key_prefix = kSectionPrefix + section + kSectionSuffix;
  keys->push_front(key_prefix);
  preference_foreach_item(callback, keys);
  keys->pop_front();
}

void PreferenceAppDB::Remove(const std::string& section,
                             const std::string& key) {
  std::string combined_key = kSectionPrefix + section + kSectionSuffix + key;
  preference_remove(combined_key.c_str());
}

#else  // end of USE_APP_PREFERENCE

SqliteDB::SqliteDB(const std::string& app_data_path)
    : app_data_path_(app_data_path),
      sqldb_(NULL) {
  if (app_data_path_.empty()) {
    std::unique_ptr<char, decltype(std::free)*>
    path {app_get_data_path(), std::free};
    if (path.get() != NULL)
      app_data_path_ = path.get();
  }
  Initialize();
}

SqliteDB::~SqliteDB() {
  if (sqldb_ != NULL) {
    sqlite3_close(sqldb_);
    sqldb_ = NULL;
  }
}

void SqliteDB::Initialize() {
  if (app_data_path_.empty()) {
    LOGGER(ERROR) << "app data path was empty";
    return;
  }
  std::string db_path = app_data_path_ + "/.appdb.db";
  int ret = sqlite3_open(db_path.c_str(), &sqldb_);
  if (ret != SQLITE_OK) {
    LOGGER(ERROR) << "Fail to open app db :" << sqlite3_errmsg(sqldb_);
    sqldb_ = NULL;
    return;
  }
  sqlite3_busy_handler(sqldb_, [](void *, int count) {
    if (count < 5) {
      LOGGER(ERROR) << "App db was busy, Wait the lock count(" << count << ")";
      usleep(100000*(count+1));
      return 1;
    } else {
      LOGGER(ERROR) << "App db was busy, Fail to access";
      return 0;
    }
  }, NULL);

  char *errmsg = NULL;
  ret = sqlite3_exec(sqldb_, kCreateDbQuery, NULL, NULL, &errmsg);
  if (ret != SQLITE_OK) {
    LOGGER(ERROR) << "Error to create appdb : " << (errmsg ? errmsg : "");
    if (errmsg)
      sqlite3_free(errmsg);
  }
}

bool SqliteDB::HasKey(const std::string& section,
                      const std::string& key) const {
  char *buffer = NULL;
  sqlite3_stmt *stmt = NULL;
  bool result = false;

  int ret = 0;
  buffer = sqlite3_mprintf(
      "select count(*) from appdb where section = %Q and key = %Q",
      section.c_str(),
      key.c_str());
  if (buffer == NULL) {
    LOGGER(ERROR) << "error to make query";
    return false;
  }

  std::unique_ptr<char, decltype(sqlite3_free)*>
      scoped_data {buffer, sqlite3_free};

  ret = sqlite3_prepare(sqldb_, buffer, strlen(buffer), &stmt, NULL);
  if (ret != SQLITE_OK) {
    LOGGER(ERROR) << "Fail to prepare query : " << sqlite3_errmsg(sqldb_);
    return false;
  }

  ret = sqlite3_step(stmt);
  if (ret == SQLITE_ROW) {
    int value = sqlite3_column_int(stmt, 0);
    result = value > 0;
  }

  sqlite3_finalize(stmt);
  return result;
}

std::string SqliteDB::Get(const std::string& section,
                          const std::string& key) const {
  char *buffer = NULL;
  sqlite3_stmt *stmt = NULL;
  std::string result;

  int ret = 0;
  buffer = sqlite3_mprintf(
      "select value from appdb where section = %Q and key = %Q",
      section.c_str(),
      key.c_str());
  if (buffer == NULL) {
    LOGGER(ERROR) << "error to make query";
    return result;
  }

  std::unique_ptr<char, decltype(sqlite3_free)*>
      scoped_data {buffer, sqlite3_free};

  ret = sqlite3_prepare(sqldb_, buffer, strlen(buffer), &stmt, NULL);
  if (ret != SQLITE_OK) {
    LOGGER(ERROR) << "Fail to prepare query : " << sqlite3_errmsg(sqldb_);
    return result;
  }

  ret = sqlite3_step(stmt);
  if (ret == SQLITE_ROW) {
    result = std::string(
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
  }

  sqlite3_finalize(stmt);
  return result;
}

void SqliteDB::Set(const std::string& section,
                   const std::string& key,
                   const std::string& value) {
  char *buffer = NULL;
  sqlite3_stmt *stmt = NULL;

  int ret = 0;
  buffer = sqlite3_mprintf(
      "replace into appdb (section, key, value) values (?, ?, ?);");
  if (buffer == NULL) {
    LOGGER(ERROR) << "error to make query";
    return;
  }

  std::unique_ptr<char, decltype(sqlite3_free)*>
      scoped_data {buffer, sqlite3_free};

  ret = sqlite3_prepare(sqldb_, buffer, strlen(buffer), &stmt, NULL);
  if (ret != SQLITE_OK) {
    LOGGER(ERROR) << "Fail to prepare query : " << sqlite3_errmsg(sqldb_);
    return;
  }

  std::unique_ptr<sqlite3_stmt, decltype(sqlite3_finalize)*>
      scoped_stmt {stmt, sqlite3_finalize};

  ret = sqlite3_bind_text(stmt,
                          1,
                          section.c_str(),
                          section.length(),
                          SQLITE_STATIC);
  if (ret != SQLITE_OK) {
    LOGGER(ERROR) << "Fail to prepare query bind argument : "
                  << sqlite3_errmsg(sqldb_);
    return;
  }
  ret = sqlite3_bind_text(stmt,
                          2,
                          key.c_str(),
                          key.length(),
                          SQLITE_STATIC);
  if (ret != SQLITE_OK) {
    LOGGER(ERROR) << "Fail to prepare query bind argument : "
                  << sqlite3_errmsg(sqldb_);
    return;
  }
  ret = sqlite3_bind_text(stmt,
                          3,
                          value.c_str(),
                          value.length(),
                          SQLITE_STATIC);
  if (ret != SQLITE_OK) {
    LOGGER(ERROR) << "Fail to prepare query bind argument : "
                  << sqlite3_errmsg(sqldb_);
    return;
  }
  ret = sqlite3_step(stmt);
  if (ret != SQLITE_DONE) {
    LOGGER(ERROR) << "Fail to insert data : " << sqlite3_errmsg(sqldb_);
  }
}

void SqliteDB::Remove(const std::string& section,
                      const std::string& key) {
  char *buffer = NULL;

  buffer = sqlite3_mprintf(
      "delete from appdb where section = %Q and key = %Q",
      section.c_str(),
      key.c_str());

  if (buffer == NULL) {
    LOGGER(ERROR) << "error to make query";
    return;
  }

  std::unique_ptr<char, decltype(sqlite3_free)*>
      scoped_data {buffer, sqlite3_free};

  char *errmsg = NULL;
  int ret = sqlite3_exec(sqldb_, buffer, NULL, NULL, &errmsg);
  if (ret != SQLITE_OK) {
    LOGGER(ERROR) << "Error to delete value : " << (errmsg ? errmsg : "");
    if (errmsg)
      sqlite3_free(errmsg);
  }
}

void SqliteDB::GetKeys(const std::string& section,
                       std::list<std::string>* keys) const {
  char *buffer = NULL;
  sqlite3_stmt *stmt = NULL;

  int ret = 0;
  buffer = sqlite3_mprintf(
      "select key from appdb where section = %Q",
      section.c_str());
  if (buffer == NULL) {
    LOGGER(ERROR) << "error to make query";
    return;
  }

  std::unique_ptr<char, decltype(sqlite3_free)*>
      scoped_data {buffer, sqlite3_free};

  ret = sqlite3_prepare(sqldb_, buffer, strlen(buffer), &stmt, NULL);
  if (ret != SQLITE_OK) {
    LOGGER(ERROR) << "Fail to prepare query : " << sqlite3_errmsg(sqldb_);
    return;
  }

  ret = sqlite3_step(stmt);
  while (ret == SQLITE_ROW) {
    const char* value =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    keys->push_back(std::string(value));
    ret = sqlite3_step(stmt);
  }

  sqlite3_finalize(stmt);
  return;
}

#endif  // end of else

AppDB* AppDB::GetInstance() {
#ifdef USE_APP_PREFERENCE
  static PreferenceAppDB instance;
#else
  static SqliteDB instance;
#endif
  return &instance;
}

}  // namespace common
