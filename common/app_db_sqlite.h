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

#ifndef XWALK_COMMON_APP_DB_SQLITE_H_
#define XWALK_COMMON_APP_DB_SQLITE_H_

#include <list>
#include <string>

#include "common/app_db.h"

class sqlite3;

namespace common {
class SqliteDB : public AppDB {
 public:
  explicit SqliteDB(const std::string& app_data_path = std::string());
  ~SqliteDB();
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

 private:
  void Initialize();
  std::string app_data_path_;
  sqlite3* sqldb_;
};

}  //  namespace common

#endif  // XWALK_COMMON_APP_DB_SQLITE_H_
