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

#include <package_manager.h>
#include <wgt_manifest_handlers/application_manifest_constants.h>
#include <wgt_manifest_handlers/widget_config_parser.h>
#include <app_manager.h>
#include <app_common.h>

#include <vector>

#include "common/file_utils.h"
#include "common/logger.h"
#include "common/profiler.h"

namespace common {

namespace {

const char* kPathSeparator = "/";
const char* kConfigXml = "config.xml";
const char* kWgtPath = "wgt";

#ifdef IME_FEATURE_SUPPORT
const char* kImeCategory = "http://tizen.org/category/ime";
#endif  // IME_FEATURE_SUPPORT
#ifdef WATCH_FACE_FEATURE_SUPPORT
const char* kIdleClockCategory = "com.samsung.wmanager.WATCH_CLOCK";
const char* kWearableClockCategory = "http://tizen.org/category/wearable_clock";
#endif  // WATCH_FACE_FEATURE_SUPPORT

}  // namespace

ApplicationData::ApplicationData(const std::string& appid)
  : app_id_(appid),
    loaded_(false) {
  SCOPE_PROFILE();
  char* res_path = app_get_resource_path();
  if (res_path != NULL) {
    application_path_ = std::string(res_path) + kWgtPath + kPathSeparator;
    free(res_path);
  }
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

const std::string ApplicationData::pkg_id() const {
  if (pkg_id_.empty()) {
    app_info_h app_info;
    int ret = app_info_create(app_id_.c_str(), &app_info);
    if (ret == APP_MANAGER_ERROR_NONE) {
      char* pkg = NULL;
      ret = app_info_get_package(app_info, &pkg);
      if (ret == APP_MANAGER_ERROR_NONE && pkg != NULL) {
        pkg_id_ = pkg;
        free(pkg);
      }
      app_info_destroy(app_info);
    }
  }
  return pkg_id_;
}

ApplicationData::AppType ApplicationData::GetAppType() {
  if (category_info_list_) {
    auto category_list = category_info_list_->categories;
    auto it = category_list.begin();
    auto end = category_list.end();
    for (; it != end; ++it) {
#ifdef IME_FEATURE_SUPPORT
      if (*it == kImeCategory) {
        return IME;
      }
#endif  // IME_FEATURE_SUPPORT
#ifdef WATCH_FACE_FEATURE_SUPPORT
      if (*it == kIdleClockCategory || *it == kWearableClockCategory) {
        return WATCH;
      }
#endif  // WATCH_FACE_FEATURE_SUPPORT
    }
  }
  return UI;
}

bool ApplicationData::LoadManifestData() {
  if (loaded_) {
    return true;
  }

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
          wgt::parse::AppControlInfo::Key()));

  category_info_list_ =
    std::static_pointer_cast<const wgt::parse::CategoryInfoList>(
      widget_config_parser->GetManifestData(
        wgt::parse::CategoryInfoList::Key()));

  meta_data_info_ =
    std::static_pointer_cast<const wgt::parse::MetaDataInfo>(
      widget_config_parser->GetManifestData(
          wgt::parse::MetaDataInfo::Key()));

  allowed_navigation_info_ =
    std::static_pointer_cast<const wgt::parse::AllowedNavigationInfo>(
      widget_config_parser->GetManifestData(
          wgt::parse::AllowedNavigationInfo::Key()));

  permissions_info_ =
    std::static_pointer_cast<const wgt::parse::PermissionsInfo>(
      widget_config_parser->GetManifestData(
        wgt::parse::PermissionsInfo::Key()));

  setting_info_ =
    std::static_pointer_cast<const wgt::parse::SettingInfo>(
      widget_config_parser->GetManifestData(
        wgt::parse::SettingInfo::Key()));

  splash_screen_info_ =
    std::static_pointer_cast<const wgt::parse::SplashScreenInfo>(
      widget_config_parser->GetManifestData(
        wgt::parse::SplashScreenInfo::Key()));

  tizen_application_info_ =
    std::static_pointer_cast<const wgt::parse::TizenApplicationInfo>(
      widget_config_parser->GetManifestData(
          wgt::parse::TizenApplicationInfo::Key()));

  widget_info_ =
    std::static_pointer_cast<const wgt::parse::WidgetInfo>(
      widget_config_parser->GetManifestData(
          wgt::parse::WidgetInfo::Key()));

  content_info_ =
    std::static_pointer_cast<const wgt::parse::ContentInfo>(
      widget_config_parser->GetManifestData(
        wgt::parse::ContentInfo::Key()));

  warp_info_ =
    std::static_pointer_cast<const wgt::parse::WarpInfo>(
      widget_config_parser->GetManifestData(
          wgt::parse::WarpInfo::Key()));

  csp_info_ =
    std::static_pointer_cast<const wgt::parse::CSPInfo>(
      widget_config_parser->GetManifestData(
          wgt::parse::CSPInfo::Key()));

  csp_report_info_ =
    std::static_pointer_cast<const wgt::parse::CSPInfo>(
      widget_config_parser->GetManifestData(
          wgt::parse::CSPInfo::Report_only_key()));

  // Set default empty object
  if (widget_info_.get() == NULL) {
    widget_info_.reset(new wgt::parse::WidgetInfo);
  }
  if (setting_info_.get() == NULL) {
    setting_info_.reset(new wgt::parse::SettingInfo);
  }

  app_type_ = GetAppType();

  loaded_ = true;

  return true;
}

// static
ApplicationDataManager* ApplicationDataManager::GetInstance() {
  static ApplicationDataManager self;
  return &self;
}

ApplicationDataManager::ApplicationDataManager() {
}

ApplicationDataManager::~ApplicationDataManager() {
}

ApplicationData* ApplicationDataManager::GetApplicationData(
    const std::string& appid) {
  auto it = cache_.find(appid);
  if (it == cache_.end()) {
    cache_[appid].reset(new ApplicationData(appid));
  }
  return cache_[appid].get();
}

}  // namespace common
