// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_APPLICATION_DATA_H_
#define WRT_COMMON_APPLICATION_DATA_H_

#include <manifest_handlers/application_icons_handler.h>
#include <manifest_handlers/application_manifest_constants.h>
#include <manifest_handlers/appwidget_handler.h>
#include <manifest_handlers/app_control_handler.h>
#include <manifest_handlers/category_handler.h>
#include <manifest_handlers/ime_handler.h>
#include <manifest_handlers/metadata_handler.h>
#include <manifest_handlers/navigation_handler.h>
#include <manifest_handlers/permissions_handler.h>
#include <manifest_handlers/service_handler.h>
#include <manifest_handlers/setting_handler.h>
#include <manifest_handlers/splash_screen_handler.h>
#include <manifest_handlers/tizen_application_handler.h>
#include <manifest_handlers/widget_handler.h>
#include <manifest_handlers/content_handler.h>


#include <memory>
#include <string>

namespace wrt {

class ApplicationData {
 public:
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

  const std::string application_path() const { return application_path_; }
  const std::string pkg_id() const { return pkg_id_; }
  const std::string app_id() const { return app_id_; }

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

  std::string application_path_;
  std::string pkg_id_;
  std::string app_id_;
};

}  // namespace wrt

#endif  // WRT_COMMON_APPLICATION_DATA_H_
