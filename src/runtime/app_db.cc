// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/app_db.h"

#include <app_preference.h>
#include <memory>

namespace wrt {

class PreferenceAppDB : public AppDB {
 public:
  PreferenceAppDB();
  virtual bool HasKey(const std::string& key) const;
  virtual std::string Get(const std::string& key) const;
  virtual void Set(const std::string& key, const std::string& value);
  virtual void GetKeys(std::list<std::string>* keys) const;
};

PreferenceAppDB::PreferenceAppDB() {
}

bool PreferenceAppDB::HasKey(const std::string& key) const {
  bool existed = false;
  return preference_is_existing(key.c_str(), &existed) == 0 && existed;
}

std::string PreferenceAppDB::Get(const std::string& key) const {
  char* value;
  if (preference_get_string(key.c_str(), &value) == 0) {
    std::unique_ptr<char, decltype(std::free)*> ptr {value, std::free};
    return std::string(value);
  }
  return std::string();
}

void PreferenceAppDB::Set(const std::string& key, const std::string& value) {
  preference_set_string(key.c_str(), value.c_str());
}

void PreferenceAppDB::GetKeys(std::list<std::string>* keys) const {
  auto callback = [](const char* key, void *user_data) {
    auto list = static_cast<std::list<std::string>*>(user_data);
    list->push_back(key);
    return true;
  };
  preference_foreach_item(callback, keys);
}

AppDB* AppDB::GetInstance() {
  static PreferenceAppDB instance;
  return &instance;
}



}  // namespace wrt
