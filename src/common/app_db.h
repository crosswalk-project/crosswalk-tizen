// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_APPDB_H_
#define WRT_RUNTIME_APPDB_H_

#include <string>
#include <list>

namespace wrt {
class AppDB {
 public:
  static AppDB* GetInstance();
  virtual bool HasKey(const std::string& section,
                      const std::string& key) const = 0;
  virtual std::string Get(const std::string& section,
                          const std::string& key) const = 0;
  virtual void Set(const std::string& section,
                   const std::string& key,
                   const std::string& value) = 0;
  virtual void GetKeys(const std::string& section,
                       std::list<std::string>* keys) const = 0;
  virtual void Remove(const std::string& section,
                      const std::string& key) = 0;
};
}  // namespace wrt
#endif  // WRT_RUNTIME_APPDB_H_
