// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wrt-upgrade/wrt-upgrade-info.h"
#include "common/logger.h"

// #include <sstream>

namespace {
const std::string kSectionPrefix = "_SECT_";
const std::string kSectionSuffix = "_SECT_";

const std::string kDBPublicSection = "public";
const std::string kDBPrivateSection = "private";

const std::string kGeolocationPermissionPrefix = "__WRT_GEOPERM_";
const std::string kNotificationPermissionPrefix = "__WRT_NOTIPERM_";
const std::string kQuotaPermissionPrefix = "__WRT_QUOTAPERM_";
const std::string kCertificateAllowPrefix = "__WRT_CERTIPERM_";
const std::string kUsermediaPermissionPrefix = "__WRT_USERMEDIAPERM_";

const std::string kValueAllow = "allowed";
const std::string kValueDenied = "denied";
const std::string kValueUnknown = "unknown";

const std::string kAppDirectoryPrefix = "/opt/usr/home/owner/apps_rw/";
const std::string kAppSecurityOriginDBFile = "/data/.security_origin.db";
const std::string kAppCertificateDBFile = "/data/.certificate.db";

enum {
  FEATURE_GEOLOCATION = 0,
  FEATURE_WEB_NOTIFICATION,
  FEATURE_USER_MEDIA,
  FEATURE_FULLSCREEN_MODE,
  FEATURE_WEB_DATABASE,
  FEATURE_CAMERA,
  FEATURE_AUDIO_RECORDER
};
enum {
  RESULT_UNKNOWN = 0,
  RESULT_ALLOW_ONCE,
  RESULT_DENY_ONCE,
  RESULT_ALLOW_ALWAYS,
  RESULT_DENY_ALWAYS
};
}  // namespace

namespace upgrade {

PreferenceInfo::PreferenceInfo(std::string key, std::string value) {
  m_section_ = kDBPublicSection;
  m_key_ = key;
  m_value_ = value;
}
SecurityOriginInfo::SecurityOriginInfo(
        int feature,
        std::string scheme,
        std::string host,
        int port,
        int result) {
  m_section_ = kDBPrivateSection;
  m_key_ = translateKey(feature, scheme, host, port);
  m_value_ = translateValue(result);
}
std::string SecurityOriginInfo::translateKey(
    int feature,
    std::string scheme,
    std::string host,
    int port) {

  std::string key = "";

  switch (feature) {
  case FEATURE_GEOLOCATION :
      key += kGeolocationPermissionPrefix;
  break;
  case FEATURE_WEB_NOTIFICATION :
      key += kNotificationPermissionPrefix;
  break;
  case FEATURE_USER_MEDIA :
      key += kUsermediaPermissionPrefix;
  break;
  case FEATURE_WEB_DATABASE :
      key += kQuotaPermissionPrefix;
  break;
  default :
  break;
  }

  key += scheme;
  key += "://";
  key += host;
  key += ":";
  key += std::to_string(port);

  return key;
}
std::string SecurityOriginInfo::translateValue(int result) {
  std::string value = "";

  switch (result) {
  case RESULT_ALLOW_ALWAYS :
       value = kValueAllow;
  break;
  case RESULT_DENY_ALWAYS :
      value = kValueDenied;
  break;
  case RESULT_UNKNOWN :
  case RESULT_ALLOW_ONCE :
  case RESULT_DENY_ONCE :
  default :
      value = kValueUnknown;
  break;
  }
  return value;
}
CertificateInfo::CertificateInfo(
        std::string pem,
        int result) {
  m_section_ = kDBPrivateSection;
  m_key_ = translateKey(pem);
  m_value_ = translateValue(result);
}
std::string CertificateInfo::translateKey(std::string pem) {
  std::string key = "";
  key = kCertificateAllowPrefix + pem;
  return key;
}
std::string CertificateInfo::translateValue(int result) {
  std::string value = "";

  switch (result) {
  case RESULT_ALLOW_ALWAYS :
    value = kValueAllow;
  break;
  case RESULT_DENY_ALWAYS :
    value = kValueDenied;
  break;
  case RESULT_UNKNOWN :
  case RESULT_ALLOW_ONCE :
  case RESULT_DENY_ONCE :
  default :
    value = kValueUnknown;
  break;
  }
  return value;
}
WrtUpgradeInfo::WrtUpgradeInfo(std::string appid) {
  app_id_ = appid;
  pkg_id_ = appid.substr(0, appid.find_first_of('.'));
  app_dir_ = kAppDirectoryPrefix + pkg_id_;
}
void WrtUpgradeInfo::addPreferenceInfo(PreferenceInfo preference) {
  preference_info_list_.push_back(preference);
}
void WrtUpgradeInfo::addSecurityOriginInfo(SecurityOriginInfo security_origin) {
  security_origin_info_list_.push_back(security_origin);
}
void WrtUpgradeInfo::addCertificateInfo(CertificateInfo certificate) {
  certificate_info_list.push_back(certificate);
}

std::string WrtUpgradeInfo::getSecurityOriginDB() {
  return app_dir_ + kAppSecurityOriginDBFile;
}
std::string WrtUpgradeInfo::getCertificateDB() {
  return app_dir_ + kAppCertificateDBFile;
}
}  // namespace upgrade
