// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wrt-upgrade/wrt-upgrade.h"

#include <sqlite3.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <utility>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/file_utils.h"

namespace {
  const std::string kWRTDbFile = "/opt/dbspace/.wrt.db";
  const std::string kAppDirectoryPrefix = "/opt/usr/home/owner/apps_rw/";
  const std::string kAppMigrationFile = "/data/.runtime.migration";
  const std::string kJournalPostfix = "-journal";
}  // namespace

namespace upgrade {
WrtUpgrade::WrtUpgrade() {
}
WrtUpgrade::~WrtUpgrade() {
}
void WrtUpgrade::Run() {
  ParseWrtDatabse();
  ParseSecurityOriginDatabase();
  ParseCertificatenDatabase();
  CreateMigrationFile();
  RemoveDatabases();
  RedirectSymlink();
}
void WrtUpgrade::ParseWrtDatabse() {
  std::cout << "parseWrtDatabse" << std::endl;
  sqlite3 *wrt_db;
  try {
    int ret = sqlite3_open(kWRTDbFile.c_str(), &wrt_db);
    if (ret != SQLITE_OK) {
      throw("error to open wrt database");
    }

    // get applist
    std::string query = "select tizen_appid from widgetinfo";
    sqlite3_stmt* stmt = NULL;

    ret = sqlite3_prepare_v2(wrt_db, query.c_str() , -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
      throw("error for prepare query");
    }

    while (SQLITE_ROW == sqlite3_step(stmt)) {
      std::string appid
        = std::string(reinterpret_cast<const char*>(
                        sqlite3_column_text(stmt, 0)));
      application_list_.push_back(appid);
      application_map_.insert(
        std::pair<std::string, WrtUpgradeInfo>(appid, WrtUpgradeInfo(appid)));
    }

    ret = sqlite3_finalize(stmt);
    if (ret != SQLITE_OK) {
      throw("error for finalize stmt");
    }

    // get widget preference
    query = "select tizen_appid,key_name,key_value from widgetpreference";
    stmt = NULL;

    ret = sqlite3_prepare_v2(wrt_db, query.c_str() , -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
      throw("error for prepare query");
    }

    while (SQLITE_ROW == sqlite3_step(stmt)) {
      std::string appid
        = std::string(reinterpret_cast<const char*>(
                       sqlite3_column_text(stmt, 0)));
      std::string key
        = std::string(reinterpret_cast<const char*>(
                       sqlite3_column_text(stmt, 1)));
      std::string value
        = std::string(reinterpret_cast<const char*>(
                       sqlite3_column_text(stmt, 2)));

      application_map_[appid].addPreferenceInfo(PreferenceInfo(key, value));
    }
    ret = sqlite3_finalize(stmt);
    if (ret != SQLITE_OK) {
      throw("error for finalize stmt");
    }
  } catch(const char* err) {
    std::cout << err << std::endl;
  }
  sqlite3_close(wrt_db);
}
void WrtUpgrade::ParseSecurityOriginDatabase() {
  std::cout << "parseSecurityOriginDatabase" << std::endl;
  for (int i = 0; i < static_cast<int>(application_list_.size()) ; i++) {
    sqlite3 *security_origin_db;
    std::string appid = application_list_[i];
    try {
      std::string db_path = application_map_[appid].getSecurityOriginDB();

      if (access(db_path.c_str(), 0) != 0) {
        throw("file is not exist : ["+db_path+"]");
      }
      int ret = sqlite3_open(db_path.c_str(), &security_origin_db);
      if (ret != SQLITE_OK) {
        throw("error to open wrt database");
      }

      // get applist
      std::string query = "select * from securityorigininfo";
      sqlite3_stmt* stmt = NULL;

      ret = sqlite3_prepare_v2(
              security_origin_db, query.c_str() , -1, &stmt, NULL);
      if (ret != SQLITE_OK) {
        throw("error for prepare query");
      }

      while (SQLITE_ROW == sqlite3_step(stmt)) {
        int feature = sqlite3_column_int(stmt, 0);
        std::string scheme
          = std::string(reinterpret_cast<const char*>(
                         sqlite3_column_text(stmt, 1)));
        std::string host
          = std::string(reinterpret_cast<const char*>(
                         sqlite3_column_text(stmt, 2)));
        int port = sqlite3_column_int(stmt, 3);
        int result = sqlite3_column_int(stmt, 4);
        application_map_[appid].addSecurityOriginInfo(
          SecurityOriginInfo(feature, scheme, host, port, result));
      }

      ret = sqlite3_finalize(stmt);
      if (ret != SQLITE_OK) {
        throw("error for finalize stmt");
      }
    } catch(...) {
    }
    sqlite3_close(security_origin_db);
  }
}
void WrtUpgrade::ParseCertificatenDatabase() {
  std::cout << "parseCertificateDatabase" << std::endl;
  for (int i = 0 ; i < static_cast<int>(application_list_.size()) ; i++) {
    sqlite3 *certificate_db;
    std::string appid = application_list_[i];
    try {
      std::string db_path = application_map_[appid].getCertificateDB();

      if (access(db_path.c_str(), 0) != 0) {
        throw("file is not exist : ["+db_path+"]");
      }
      int ret = sqlite3_open(db_path.c_str(), &certificate_db);
      if (ret != SQLITE_OK) {
        throw("error to open wrt database");
      }

      // get applist
      std::string query = "select * from certificateinfo";
      sqlite3_stmt* stmt = NULL;

      ret = sqlite3_prepare_v2(certificate_db, query.c_str() , -1, &stmt, NULL);
      if (ret != SQLITE_OK) {
        throw("error for prepare query");
      }

      while (SQLITE_ROW == sqlite3_step(stmt)) {
        std::string certificate
          = std::string(reinterpret_cast<const char*>(
                         sqlite3_column_text(stmt, 0)));
        int result = sqlite3_column_int(stmt, 1);
        application_map_[appid].addCertificateInfo(
                                  CertificateInfo(certificate, result));
      }

      ret = sqlite3_finalize(stmt);
      if (ret != SQLITE_OK) {
        throw("error for finalize stmt");
      }
    } catch(...) {
    }
    sqlite3_close(certificate_db);
  }
}
void WrtUpgrade::CreateMigrationFile() {
  std::cout << "createMigrationFile" << std::endl;
  for (int i = 0 ; i < static_cast<int>(application_list_.size()) ; i++) {
    std::string appid = application_list_[i];
    std::string json_str = CreateJsonObject(appid);
    if (!json_str.empty()) {
      std::string pkg_id = appid.substr(0, appid.find_first_of('.'));
      std::string output_file_path
        = kAppDirectoryPrefix + pkg_id + kAppMigrationFile;
      std::ofstream output_file(output_file_path);
      output_file << json_str;
      output_file.close();
    }
  }
}
void WrtUpgrade::RemoveDatabases() {
  for (int i = 0 ; i < static_cast<int>(application_list_.size()) ; i++) {
    std::string appid = application_list_[i];
    try {
      std::string security_origin_db_path
        = application_map_[appid].getSecurityOriginDB();
      std::string security_origin_journal_db_path
        = security_origin_db_path + kJournalPostfix;
      std::string certificate_db_path
        = application_map_[appid].getCertificateDB();
      std::string certificate_journal_db_path
        = certificate_db_path + kJournalPostfix;

      RemoveFile(security_origin_db_path);
      RemoveFile(security_origin_journal_db_path);
      RemoveFile(certificate_db_path);
      RemoveFile(certificate_journal_db_path);
    } catch(...) {
    }
  }
}
std::string WrtUpgrade::CreateJsonObject(std::string appid) {
  WrtUpgradeInfo info = application_map_[appid];

  if (info.getPreferenceInfoSize() == 0
      && info.getSecurityOriginInfoSize() == 0
      && info.getCertificateInfoSize() == 0) {
    return "";
  }
  std::cout << "createJsonObject for " << appid << std::endl;

  // make preference value arr : vector
  picojson::array preference_arr;
  for (int i = 0 ; i < info.getPreferenceInfoSize() ; i++) {
    PreferenceInfo p_info = info.getPreferenceInfo(i);
    picojson::object obj;
    obj["key"] = picojson::value(p_info.getKey());
    obj["value"] = picojson::value(p_info.getValue());
    preference_arr.push_back(picojson::value(obj));
  }

  // make security_origin value arr : vector
  picojson::array securityorigin_arr;
  for (int i = 0 ; i < info.getSecurityOriginInfoSize() ; i++) {
    SecurityOriginInfo s_info = info.getSecurityOriginInfo(i);
    picojson::object obj;
    obj["section"] = picojson::value(s_info.getSection());
    obj["key"] = picojson::value(s_info.getKey());
    obj["value"] = picojson::value(s_info.getValue());
    securityorigin_arr.push_back(picojson::value(obj));
  }

  // make certificate value arr : vector
  picojson::array certificate_arr;
  for (int i = 0 ; i < info.getCertificateInfoSize() ; i++) {
    CertificateInfo c_info = info.getCertificateInfo(i);
    picojson::object obj;
    obj["section"] = picojson::value(c_info.getSection());
    obj["key"] = picojson::value(c_info.getKey());
    obj["value"] = picojson::value(c_info.getValue());
    certificate_arr.push_back(picojson::value(obj));
  }

  // make one json
  picojson::object migration_obj;
  migration_obj["preference"] = picojson::value(preference_arr);
  migration_obj["security_orgin"] = picojson::value(securityorigin_arr);
  migration_obj["certificate"] = picojson::value(certificate_arr);

  picojson::value migration_val = picojson::value(migration_obj);
  std::string json_str = migration_val.serialize();
  return json_str;
}
bool WrtUpgrade::RemoveFile(const std::string& path) {
  if (!common::utils::Exists(path)) {
    LOGGER(ERROR) << "File is not Exist : " << path;
    return false;
  }
  return (remove(path.c_str()) == 0);
}
void WrtUpgrade::RedirectSymlink() {
  std::string xwalk_runtime = "/usr/bin/xwalk_runtime";
  std::string wrt_client = "/usr/bin/wrt-client";

  symlink(xwalk_runtime.c_str(), wrt_client.c_str());
}
}  // namespace upgrade
int main() {
  std::cout << "Tizen database migration tool for Webruntime" << std::endl;
  upgrade::WrtUpgrade mt;
  mt.Run();
  return 0;
}
