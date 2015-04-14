// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/application_data.h"

#include <package_manager.h>
#include <manifest_parser/manifest_parser.h>
#include <manifest_parser/manifest_handler.h>

#include <vector>

namespace wrt {

namespace {

const char* kPathSeparator = "/";
const char* kConfigXml = "config.xml";

static std::string GetPackageIdByAppId(const std::string& appid) {
  char* pkgid = NULL;
  package_manager_get_package_id_by_app_id(appid.c_str(), &pkgid);

  std::unique_ptr<char, decltype(std::free)*>
    pkgid_ptr {pkgid, std::free};

  if (pkgid != NULL) {
    return std::string(pkgid_ptr.get());
  } else {
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
    return std::string();
  }
}

}  // namespace

ApplicationData::ApplicationData(const std::string& appid) : app_id_(appid) {
  pkg_id_ = GetPackageIdByAppId(appid);

  if (!pkg_id_.empty()) {
    pkg_root_path_ = GetPackageRootPath(pkg_id_);
  }

  if (!pkg_root_path_.empty()) {
    config_xml_path_ = pkg_root_path_ + kPathSeparator +
                       app_id_ + kPathSeparator + kConfigXml;
  }

  LoadManifestData();
}

ApplicationData::~ApplicationData() {}

std::shared_ptr<const wgt::parse::ApplicationIconsInfo>
    ApplicationData::application_icons_info() const {
  return application_icons_info_;
}

std::shared_ptr<const wgt::parse::AppWidgetInfo>
    ApplicationData::app_widget_info() const {
  return app_widget_info_;
}

std::shared_ptr<const wgt::parse::AppControlInfoList>
    ApplicationData::app_control_info_list() const {
  return app_control_info_list_;
}

std::shared_ptr<const wgt::parse::CategoryInfoList>
    ApplicationData::category_info_list() const {
  return category_info_list_;
}

std::shared_ptr<const wgt::parse::ImeInfo>
    ApplicationData::ime_info() const {
  return ime_info_;
}

std::shared_ptr<const wgt::parse::MetaDataInfo>
    ApplicationData::meta_data_info() const {
  return meta_data_info_;
}

std::shared_ptr<const wgt::parse::NavigationInfo>
    ApplicationData::navigation_info() const {
  return navigation_info_;
}

std::shared_ptr<const wgt::parse::PermissionsInfo>
    ApplicationData::permissions_info() const {
  return permissions_info_;
}

std::shared_ptr<const wgt::parse::ServiceList>
    ApplicationData::service_list() const {
  return service_list_;
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

bool ApplicationData::LoadManifestData() {
  if (config_xml_path_.empty()) {
    return false;
  }

  enum ManifestHandlerType {
    APPLICATION_ICONS_HANDLER = 0,
    APP_WIDGET_HANDLER,
    APP_CONTROL_HANDLER,
    CATEGORY_HANDLER,
    IME_HANDLER,
    META_DATA_HANDLER,
    NAVIGATION_HANDLER,
    PERMISSIONS_HANDLER,
    SERVICE_HANDLER,
    SETTING_HANDLER,
    SPLASH_SCREEN_HANDLER,
    TIZEN_APPLICATION_HANDLER,
    WIDGET_HANDLER
  };

  std::vector<parser::ManifestHandler*> handlers = {
    new wgt::parse::ApplicationIconsHandler,  // APPLICATION_ICONS_HANDLER
    new wgt::parse::AppWidgetHandler,         // APP_WIDGET_HANDLER
    new wgt::parse::AppControlHandler,        // APP_CONTROL_HANDLER
    new wgt::parse::CategoryHandler,          // CATEGORY_HANDLER
    new wgt::parse::ImeHandler,               // IME_HANDLER
    new wgt::parse::MetaDataHandler,          // META_DATA_HANDLER
    new wgt::parse::NavigationHandler,        // NAVIGATION_HANDLER
    new wgt::parse::PermissionsHandler,       // PERMISSIONS_HANDLER
    new wgt::parse::ServiceHandler,           // SERVICE_HANDLER
    new wgt::parse::SettingHandler,           // SETTING_HANDLER
    new wgt::parse::SplashScreenHandler,      // SPLASH_SCREEN_HANDLER
    new wgt::parse::TizenApplicationHandler,  // TIZEN_APPLICATION_HANDLER
    new wgt::parse::WidgetHandler             // WIDGET_HANDLER
  };

  std::unique_ptr<parser::ManifestHandlerRegistry> registry;
  registry.reset(new parser::ManifestHandlerRegistry(handlers));

  parser::ManifestParser manifest_parser(std::move(registry));
  if (!manifest_parser.ParseManifest(config_xml_path_)) {
    for (auto iter = handlers.begin(); iter != handlers.end(); ++iter) {
      delete *iter;
    }
    return false;
  }

  application_icons_info_ =
    std::static_pointer_cast<const wgt::parse::ApplicationIconsInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::APPLICATION_ICONS_HANDLER]->Key()));

  app_widget_info_ =
    std::static_pointer_cast<const wgt::parse::AppWidgetInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::APP_WIDGET_HANDLER]->Key()));

  app_control_info_list_ =
    std::static_pointer_cast<const wgt::parse::AppControlInfoList>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::APP_CONTROL_HANDLER]->Key()));

  category_info_list_ =
    std::static_pointer_cast<const wgt::parse::CategoryInfoList>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::CATEGORY_HANDLER]->Key()));

  ime_info_ =
    std::static_pointer_cast<const wgt::parse::ImeInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::IME_HANDLER]->Key()));

  meta_data_info_ =
    std::static_pointer_cast<const wgt::parse::MetaDataInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::META_DATA_HANDLER]->Key()));

  navigation_info_ =
    std::static_pointer_cast<const wgt::parse::NavigationInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::NAVIGATION_HANDLER]->Key()));

  permissions_info_ =
    std::static_pointer_cast<const wgt::parse::PermissionsInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::PERMISSIONS_HANDLER]->Key()));

  service_list_ =
    std::static_pointer_cast<const wgt::parse::ServiceList>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::SERVICE_HANDLER]->Key()));

  setting_info_ =
    std::static_pointer_cast<const wgt::parse::SettingInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::SETTING_HANDLER]->Key()));

  splash_screen_info_ =
    std::static_pointer_cast<const wgt::parse::SplashScreenInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::SPLASH_SCREEN_HANDLER]->Key()));

  tizen_application_info_ =
    std::static_pointer_cast<const wgt::parse::TizenApplicationInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::TIZEN_APPLICATION_HANDLER]->Key()));

  widget_info_ =
    std::static_pointer_cast<const wgt::parse::WidgetInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::WIDGET_HANDLER]->Key()));

  for (auto iter = handlers.begin(); iter != handlers.end(); ++iter) {
    delete *iter;
  }

  return true;
}

}  // namespace wrt
