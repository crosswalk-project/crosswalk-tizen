// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/application_data.h"

#include <package_manager.h>
#include <manifest_parser/manifest_parser.h>
#include <manifest_parser/manifest_handler.h>

#include <vector>

#include "common/logger.h"
#include "common/file_utils.h"

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
    application_path_ = GetPackageRootPath(pkg_id_) + kPathSeparator +
                        appid + kPathSeparator;
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


bool ApplicationData::LoadManifestData() {
  std::string config_xml_path(application_path_ + kConfigXml);
  if (!utils::Exists(config_xml_path)) {
    LOGGER(ERROR) << "Failed to load manifest data. : No such file '"
                  << config_xml_path << "'.";
    return false;
  }

  enum ManifestHandlerType {
    APP_CONTROL_HANDLER = 0,
    CATEGORY_HANDLER,
    META_DATA_HANDLER,
    NAVIGATION_HANDLER,
    PERMISSIONS_HANDLER,
    SETTING_HANDLER,
    SPLASH_SCREEN_HANDLER,
    TIZEN_APPLICATION_HANDLER,
    WIDGET_HANDLER,
    CONTENT_HANDLER
  };

  std::vector<parser::ManifestHandler*> handlers = {
    new wgt::parse::AppControlHandler,        // APP_CONTROL_HANDLER
    new wgt::parse::CategoryHandler,          // CATEGORY_HANDLER
    new wgt::parse::MetaDataHandler,          // META_DATA_HANDLER
    new wgt::parse::NavigationHandler,        // NAVIGATION_HANDLER
    new wgt::parse::PermissionsHandler,       // PERMISSIONS_HANDLER
    new wgt::parse::SettingHandler,           // SETTING_HANDLER
    new wgt::parse::SplashScreenHandler,      // SPLASH_SCREEN_HANDLER
    new wgt::parse::TizenApplicationHandler,  // TIZEN_APPLICATION_HANDLER
    new wgt::parse::WidgetHandler,            // WIDGET_HANDLER
    new wgt::parse::ContentHandler            // CONTENT_HANDLER
  };

  std::unique_ptr<parser::ManifestHandlerRegistry> registry;
  registry.reset(new parser::ManifestHandlerRegistry(handlers));

  parser::ManifestParser manifest_parser(std::move(registry));
  if (!manifest_parser.ParseManifest(config_xml_path)) {
    for (auto iter = handlers.begin(); iter != handlers.end(); ++iter) {
      delete *iter;
    }
    LOGGER(ERROR) << "Failed to load manifest data : "
                  << manifest_parser.GetErrorMessage();
    return false;
  }

  app_control_info_list_ =
    std::static_pointer_cast<const wgt::parse::AppControlInfoList>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::APP_CONTROL_HANDLER]->Key()));

  category_info_list_ =
    std::static_pointer_cast<const wgt::parse::CategoryInfoList>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::CATEGORY_HANDLER]->Key()));

  meta_data_info_ =
    std::static_pointer_cast<const wgt::parse::MetaDataInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::META_DATA_HANDLER]->Key()));

  allowed_navigation_info_ =
    std::static_pointer_cast<const wgt::parse::AllowedNavigationInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::NAVIGATION_HANDLER]->Key()));

  permissions_info_ =
    std::static_pointer_cast<const wgt::parse::PermissionsInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::PERMISSIONS_HANDLER]->Key()));

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

  content_info_ =
    std::static_pointer_cast<const wgt::parse::ContentInfo>(
      manifest_parser.GetManifestData(
        handlers[ManifestHandlerType::CONTENT_HANDLER]->Key()));

  for (auto iter = handlers.begin(); iter != handlers.end(); ++iter) {
    delete *iter;
  }

  // Set default empty object
  if (widget_info_.get() == NULL) {
    widget_info_.reset(new wgt::parse::WidgetInfo);
  }
  if (setting_info_.get() == NULL) {
    setting_info_.reset(new wgt::parse::SettingInfo);
  }

  return true;
}

}  // namespace wrt
