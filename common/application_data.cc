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

#include "common/application_data.h"

#include <manifest_handlers/application_manifest_constants.h>
#include <manifest_handlers/widget_config_parser.h>
#include <package_manager.h>

#include <vector>

#include "common/file_utils.h"
#include "common/logger.h"
#include "common/profiler.h"

namespace common {

namespace {

const char* kPathSeparator = "/";
const char* kConfigXml = "config.xml";
const char* kResWgtPath = "res/wgt";

static std::string GetPackageIdByAppId(const std::string& appid) {
  char* pkgid = NULL;
  package_manager_get_package_id_by_app_id(appid.c_str(), &pkgid);

  std::unique_ptr<char, decltype(std::free)*>
    pkgid_ptr {pkgid, std::free};

  if (pkgid != NULL) {
    return std::string(pkgid_ptr.get());
  } else {
    LOGGER(ERROR) << "Failed to get package id";
    return std::string();
  }
}

static std::string GetPackageRootPath(const std::string& pkgid) {
  package_info_h pkg_info = NULL;
  if (package_manager_get_package_info(
        pkgid.c_str(), &pkg_info) != PACKAGE_MANAGER_ERROR_NONE) {
    return std::string();
  }

  char* pkg_root_path = NULL;
  package_info_get_root_path(pkg_info, &pkg_root_path);

  std::unique_ptr<char, decltype(std::free)*>
    path_ptr {pkg_root_path, std::free};

  package_info_destroy(pkg_info);

  if (pkg_root_path != NULL) {
    return std::string(path_ptr.get());
  } else {
    LOGGER(ERROR) << "Failed to get package root path";
    return std::string();
  }
}

}  // namespace

ApplicationData::ApplicationData(const std::string& appid) : app_id_(appid) {
  pkg_id_ = GetPackageIdByAppId(appid);
  if (!pkg_id_.empty())
    application_path_ = GetPackageRootPath(pkg_id_) + kPathSeparator
                        + kResWgtPath + kPathSeparator;
}

ApplicationData::~ApplicationData() {}

std::shared_ptr<const wgt::parse::AppControlInfoList>
    ApplicationData::app_control_info_list() const {
  return app_control_info_list_;
}

std::shared_ptr<const wgt::parse::CategoryInfoList>
    ApplicationData::category_info_list() const {
  return category_info_list_;
}

std::shared_ptr<const wgt::parse::MetaDataInfo>
    ApplicationData::meta_data_info() const {
  return meta_data_info_;
}

std::shared_ptr<const wgt::parse::AllowedNavigationInfo>
    ApplicationData::allowed_navigation_info() const {
  return allowed_navigation_info_;
}

std::shared_ptr<const wgt::parse::PermissionsInfo>
    ApplicationData::permissions_info() const {
  return permissions_info_;
}

std::shared_ptr<const wgt::parse::SettingInfo>
    ApplicationData::setting_info() const {
  return setting_info_;
}

std::shared_ptr<const wgt::parse::SplashScreenInfo>
    ApplicationData::splash_screen_info() const {
  return splash_screen_info_;
}

std::shared_ptr<const wgt::parse::TizenApplicationInfo>
    ApplicationData::tizen_application_info() const {
  return tizen_application_info_;
}

std::shared_ptr<const wgt::parse::WidgetInfo>
    ApplicationData::widget_info() const {
  return widget_info_;
}

std::shared_ptr<const wgt::parse::ContentInfo>
    ApplicationData::content_info() const {
  return content_info_;
}

std::shared_ptr<const wgt::parse::WarpInfo>
    ApplicationData::warp_info() const {
  return warp_info_;
}

std::shared_ptr<const wgt::parse::CSPInfo>
    ApplicationData::csp_info() const {
  return csp_info_;
}

std::shared_ptr<const wgt::parse::CSPInfo>
    ApplicationData::csp_report_info() const {
  return csp_report_info_;
}


bool ApplicationData::LoadManifestData() {
  SCOPE_PROFILE();
  std::string config_xml_path(application_path_ + kConfigXml);
  if (!utils::Exists(config_xml_path)) {
    LOGGER(ERROR) << "Failed to load manifest data : No such file '"
                  << config_xml_path << "'.";
    return false;
  }

  std::unique_ptr<wgt::parse::WidgetConfigParser> widget_config_parser;
  widget_config_parser.reset(new wgt::parse::WidgetConfigParser());
  if (!widget_config_parser->ParseManifest(config_xml_path)) {
    LOGGER(ERROR) << "Failed to load widget config parser data: "
                  << widget_config_parser->GetErrorMessage();
    return false;
  }

  app_control_info_list_ =
    std::static_pointer_cast<const wgt::parse::AppControlInfoList>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kTizenApplicationAppControlsKey));

  category_info_list_ =
    std::static_pointer_cast<const wgt::parse::CategoryInfoList>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kTizenCategoryKey));

  meta_data_info_ =
    std::static_pointer_cast<const wgt::parse::MetaDataInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kTizenMetaDataKey));

  allowed_navigation_info_ =
    std::static_pointer_cast<const wgt::parse::AllowedNavigationInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kAllowNavigationKey));

  permissions_info_ =
    std::static_pointer_cast<const wgt::parse::PermissionsInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kTizenPermissionsKey));

  setting_info_ =
    std::static_pointer_cast<const wgt::parse::SettingInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kTizenSettingKey));

  splash_screen_info_ =
    std::static_pointer_cast<const wgt::parse::SplashScreenInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kTizenSplashScreenKey));

  tizen_application_info_ =
    std::static_pointer_cast<const wgt::parse::TizenApplicationInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kTizenApplicationKey));

  widget_info_ =
    std::static_pointer_cast<const wgt::parse::WidgetInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kTizenWidgetKey));

  content_info_ =
    std::static_pointer_cast<const wgt::parse::ContentInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kTizenContentKey));

  warp_info_ =
    std::static_pointer_cast<const wgt::parse::WarpInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kAccessKey));

  csp_info_ =
    std::static_pointer_cast<const wgt::parse::CSPInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kCSPKey));

  csp_report_info_ =
    std::static_pointer_cast<const wgt::parse::CSPInfo>(
      widget_config_parser->GetManifestData(
        wgt::application_widget_keys::kCSPReportOnlyKey));

  // Set default empty object
  if (widget_info_.get() == NULL) {
    widget_info_.reset(new wgt::parse::WidgetInfo);
  }
  if (setting_info_.get() == NULL) {
    setting_info_.reset(new wgt::parse::SettingInfo);
  }

  return true;
}

}  // namespace common
