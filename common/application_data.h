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

#ifndef XWALK_COMMON_APPLICATION_DATA_H_
#define XWALK_COMMON_APPLICATION_DATA_H_

#include <wgt_manifest_handlers/application_icons_handler.h>
#include <wgt_manifest_handlers/appwidget_handler.h>
#include <wgt_manifest_handlers/app_control_handler.h>
#include <wgt_manifest_handlers/category_handler.h>
#include <wgt_manifest_handlers/content_handler.h>
#include <wgt_manifest_handlers/csp_handler.h>
#include <wgt_manifest_handlers/ime_handler.h>
#include <wgt_manifest_handlers/metadata_handler.h>
#include <wgt_manifest_handlers/navigation_handler.h>
#include <wgt_manifest_handlers/permissions_handler.h>
#include <wgt_manifest_handlers/service_handler.h>
#include <wgt_manifest_handlers/setting_handler.h>
#include <wgt_manifest_handlers/splash_screen_handler.h>
#include <wgt_manifest_handlers/tizen_application_handler.h>
#include <wgt_manifest_handlers/warp_handler.h>
#include <wgt_manifest_handlers/widget_handler.h>

#include <memory>
#include <string>

namespace common {

class ApplicationData {
 public:
  enum AppType {
    UI = 0
#ifdef IME_FEATURE_SUPPORT
    ,IME
#endif  // IME_FEATURE_SUPPORT
  };

  explicit ApplicationData(const std::string& appid);
  ~ApplicationData();

  bool LoadManifestData();

  std::shared_ptr<const wgt::parse::AppControlInfoList>
    app_control_info_list() const;
  std::shared_ptr<const wgt::parse::CategoryInfoList>
    category_info_list() const;
  std::shared_ptr<const wgt::parse::MetaDataInfo>
    meta_data_info() const;
  std::shared_ptr<const wgt::parse::AllowedNavigationInfo>
    allowed_navigation_info() const;
  std::shared_ptr<const wgt::parse::PermissionsInfo>
    permissions_info() const;
  std::shared_ptr<const wgt::parse::SettingInfo>
    setting_info() const;
  std::shared_ptr<const wgt::parse::SplashScreenInfo>
    splash_screen_info() const;
  std::shared_ptr<const wgt::parse::TizenApplicationInfo>
    tizen_application_info() const;
  std::shared_ptr<const wgt::parse::WidgetInfo>
    widget_info() const;
  std::shared_ptr<const wgt::parse::ContentInfo>
    content_info() const;
  std::shared_ptr<const wgt::parse::WarpInfo>
    warp_info() const;
  std::shared_ptr<const wgt::parse::CSPInfo>
    csp_info() const;
  std::shared_ptr<const wgt::parse::CSPInfo>
    csp_report_info() const;

  const std::string application_path() const { return application_path_; }
  const std::string pkg_id() const { return pkg_id_; }
  const std::string app_id() const { return app_id_; }
  ApplicationData::AppType app_type() { return app_type_; }

 private:
  std::shared_ptr<const wgt::parse::AppControlInfoList>
    app_control_info_list_;
  std::shared_ptr<const wgt::parse::CategoryInfoList>
    category_info_list_;
  std::shared_ptr<const wgt::parse::MetaDataInfo>
    meta_data_info_;
  std::shared_ptr<const wgt::parse::AllowedNavigationInfo>
    allowed_navigation_info_;
  std::shared_ptr<const wgt::parse::PermissionsInfo>
    permissions_info_;
  std::shared_ptr<const wgt::parse::SettingInfo>
    setting_info_;
  std::shared_ptr<const wgt::parse::SplashScreenInfo>
    splash_screen_info_;
  std::shared_ptr<const wgt::parse::TizenApplicationInfo>
    tizen_application_info_;
  std::shared_ptr<const wgt::parse::WidgetInfo>
    widget_info_;
  std::shared_ptr<const wgt::parse::ContentInfo>
    content_info_;
  std::shared_ptr<const wgt::parse::WarpInfo>
    warp_info_;
  std::shared_ptr<const wgt::parse::CSPInfo>
    csp_info_;
  std::shared_ptr<const wgt::parse::CSPInfo>
    csp_report_info_;
  ApplicationData::AppType GetAppType();

  std::string application_path_;
  std::string pkg_id_;
  std::string app_id_;
  ApplicationData::AppType app_type_;
};


class ApplicationDataManager {
 public:
  static ApplicationDataManager* GetInstance();

  ApplicationData* GetApplicationData(const std::string& appid);

 private:
  ApplicationDataManager();
  virtual ~ApplicationDataManager();

  std::map<std::string, std::unique_ptr<ApplicationData>> cache_;
};


}  // namespace common

#endif  // XWALK_COMMON_APPLICATION_DATA_H_
