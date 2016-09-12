// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_UPGRADE_H
#define WRT_UPGRADE_H
#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "wrt-upgrade/wrt-upgrade-info.h"

namespace upgrade {
class WrtUpgrade{
 public:
  WrtUpgrade();
  ~WrtUpgrade();
  void Run();
 private:
  void ParseWrtDatabse();
  void ParseSecurityOriginDatabase();
  void ParseCertificatenDatabase();
  void CreateMigrationFile();
  void RemoveDatabases();
  bool RemoveFile(const std::string& path);
  void RedirectSymlink();
  std::string CreateJsonObject(std::string appid);
  std::vector<std::string> application_list_;
  std::map<std::string, WrtUpgradeInfo> application_map_;
};
}  // namespace upgrade
#endif  // WRT_UPGRADE_H
