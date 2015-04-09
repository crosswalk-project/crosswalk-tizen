// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_APPLICATION_DATA_H_
#define WRT_RUNTIME_APPLICATION_DATA_H_

#include <memory>
#include <string>

namespace wgt {

namespace parse {

class ApplicationIconsInfo;
class AppWidgetInfo;
struct AppControlInfoList;
struct CategoryInfoList;
class ImeInfo;
class MetaDataInfo;
class NavigationInfo;
class PermissionsInfo;
struct ServiceList;
class SettingInfo;
class SplashScreenInfo;
class TizenApplicationInfo;
class WidgetInfo;

}  // namespace parse

}  // namespace wgt

namespace wrt {

class ApplicationData {
 public:
  explicit ApplicationData(const std::string& appid);
  ~ApplicationData();

  std::shared_ptr<const wgt::parse::ApplicationIconsInfo>
  application_icons_info() const {
    return application_icons_info_;
  }

  std::shared_ptr<const wgt::parse::AppWidgetInfo> app_widget_info() const {
    return app_widget_info_;
  }

  std::shared_ptr<const wgt::parse::AppControlInfoList>
  app_control_info() const {
    return app_control_info_list_;
  }

  std::shared_ptr<const wgt::parse::CategoryInfoList>
  category_info_list() const {
    return category_info_list_;
  }

  std::shared_ptr<const wgt::parse::ImeInfo> ime_info() const {
    return ime_info_;
  }

  std::shared_ptr<const wgt::parse::MetaDataInfo> meta_data_info() const {
    return meta_data_info_;
  }

  std::shared_ptr<const wgt::parse::NavigationInfo> navigation_info() const {
    return navigation_info_;
  }

  std::shared_ptr<const wgt::parse::PermissionsInfo>
  permissions_info() const {
    return permissions_info_;
  }

  std::shared_ptr<const wgt::parse::ServiceList> service_list() const {
    return service_list_;
  }

  std::shared_ptr<const wgt::parse::SettingInfo> setting_info() const {
    return setting_info_;
  }

  std::shared_ptr<const wgt::parse::SplashScreenInfo>
  splash_screen_info() const {
    return splash_screen_info_;
  }

  std::shared_ptr<const wgt::parse::TizenApplicationInfo>
  tizen_application_info() const {
    return tizen_application_info_;
  }

  std::shared_ptr<const wgt::parse::WidgetInfo> widget_info() const {
    return widget_info_;
  }

  std::string config_xml_path() const { return config_xml_path_; }
  const std::string pkg_root_path() const { return pkg_root_path_; }
  const std::string pkg_id() const { return pkg_id_; }
  const std::string app_id() const { return app_id_; }

 private:
  bool LoadManifestData();

  std::shared_ptr<const wgt::parse::ApplicationIconsInfo>
    application_icons_info_;
  std::shared_ptr<const wgt::parse::AppWidgetInfo> app_widget_info_;
  std::shared_ptr<const wgt::parse::AppControlInfoList>
    app_control_info_list_;
  std::shared_ptr<const wgt::parse::CategoryInfoList> category_info_list_;
  std::shared_ptr<const wgt::parse::ImeInfo> ime_info_;
  std::shared_ptr<const wgt::parse::MetaDataInfo> meta_data_info_;
  std::shared_ptr<const wgt::parse::NavigationInfo> navigation_info_;
  std::shared_ptr<const wgt::parse::PermissionsInfo> permissions_info_;
  std::shared_ptr<const wgt::parse::ServiceList> service_list_;
  std::shared_ptr<const wgt::parse::SettingInfo> setting_info_;
  std::shared_ptr<const wgt::parse::SplashScreenInfo> splash_screen_info_;
  std::shared_ptr<const wgt::parse::TizenApplicationInfo>
    tizen_application_info_;
  std::shared_ptr<const wgt::parse::WidgetInfo> widget_info_;

  std::string config_xml_path_;
  std::string pkg_root_path_;
  std::string pkg_id_;
  std::string app_id_;
};

}  // namespace wrt

#endif  // WRT_RUNTIME_APPLICATION_DATA_H_
