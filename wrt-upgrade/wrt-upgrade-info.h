// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_UPGRADE_INFO_H
#define WRT_UPGRADE_INFO_H

#include <iostream>
#include <string>
#include <vector>

namespace upgrade {
class PreferenceInfo{
 public:
  PreferenceInfo(std::string key, std::string value);
  std::string getSection() {return m_section_;}
  std::string getKey() {return m_key_;}
  std::string getValue() {return m_value_;}
 private:
  std::string m_section_;
  std::string m_key_;
  std::string m_value_;
};

class SecurityOriginInfo{
 public:
    SecurityOriginInfo(
         int feature,
        std::string scheme,
        std::string host,
        int port,
        int result);
  std::string getSection() {return m_section_;}
  std::string getKey() {return m_key_;}
  std::string getValue() {return m_value_;}
 private:
  std::string translateKey(
     int feature,
      std::string scheme,
      std::string host,
      int port);
  std::string translateValue(int result);
  std::string m_section_;
  std::string m_key_;
  std::string m_value_;
};

class CertificateInfo{
 public:
    CertificateInfo(
        std::string pem,
        int result);
  std::string getSection() {return m_section_;}
  std::string getKey() {return m_key_;}
  std::string getValue() {return m_value_;}
 private:
  std::string translateKey(std::string pem);
  std::string translateValue(int result);
  std::string m_section_;
  std::string m_key_;
  std::string m_value_;
};

class WrtUpgradeInfo{
 public:
  WrtUpgradeInfo() {}
  explicit WrtUpgradeInfo(std::string appid);

  std::string getAppid() { return app_id_; }
  std::string getPkgid() { return pkg_id_; }
  std::string getAppDir() { return app_dir_; }
  std::string getSecurityOriginDB();
  std::string getCertificateDB();

  void addPreferenceInfo(PreferenceInfo preference);
  void addSecurityOriginInfo(SecurityOriginInfo security_origin);
  void addCertificateInfo(CertificateInfo certificate);

  PreferenceInfo getPreferenceInfo(int idx)
      {return preference_info_list_[idx];}
  SecurityOriginInfo getSecurityOriginInfo(int idx)
      {return security_origin_info_list_[idx];}
  CertificateInfo getCertificateInfo(int idx)
      {return certificate_info_list[idx];}

  int getPreferenceInfoSize()
      {return static_cast<int>(preference_info_list_.size());}
  int getSecurityOriginInfoSize()
      {return static_cast<int>(security_origin_info_list_.size());}
  int getCertificateInfoSize()
      {return static_cast<int>(certificate_info_list.size());}

 private:
  std::string pkg_id_;
  std::string app_id_;
  std::string app_dir_;
  std::vector<PreferenceInfo> preference_info_list_;
  std::vector<SecurityOriginInfo> security_origin_info_list_;
  std::vector<CertificateInfo> certificate_info_list;
};
}  // namespace upgrade
#endif  // WRT_UPGRADE_INFO_H
