// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/common/xwalk_extension_manager.h"

#include <glob.h>
#include <dlfcn.h>

#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "common/app_db.h"
#include "common/logger.h"
#include "common/picojson.h"
#include "common/file_utils.h"
#include "common/string_utils.h"

#include "extensions/common/constants.h"
#include "extensions/common/xwalk_extension.h"

#ifndef EXTENSION_PATH
  #error EXTENSION_PATH is not set.
#endif

namespace extensions {

namespace {

const char kAppDBRuntimeSection[] = "Runtime";

const char kExtensionPrefix[] = "lib";
const char kExtensionSuffix[] = ".so";
const char kExtensionMetadataSuffix[] = ".json";

static const char* kPreloadLibs[] = {
  EXTENSION_PATH"/libtizen.so",
  EXTENSION_PATH"/libtizen_common.so",
  EXTENSION_PATH"/libtizen_application.so",
  EXTENSION_PATH"/libtizen_utils.so",
  NULL
};

}  // namespace

XWalkExtensionManager::XWalkExtensionManager() {
}

XWalkExtensionManager::~XWalkExtensionManager() {
}

void XWalkExtensionManager::PreloadExtensions() {
  for (int i = 0; kPreloadLibs[i]; i++) {
    LOGGER(DEBUG) << "Preload libs : " << kPreloadLibs[i];
    void* handle = dlopen(kPreloadLibs[i], RTLD_NOW|RTLD_GLOBAL);
    if (handle == nullptr) {
      LOGGER(WARN) << "Fail to load libs : " << dlerror();
    }
  }
}

void XWalkExtensionManager::LoadExtensions(bool meta_only) {
  if (!extensions_.empty()) {
    return;
  }

  std::string extension_path(EXTENSION_PATH);

  // Gets all extension files in the EXTENSION_PATH
  std::string ext_pattern(extension_path);
  ext_pattern.append("/");
  ext_pattern.append(kExtensionPrefix);
  ext_pattern.append("*");
  ext_pattern.append(kExtensionSuffix);

  StringSet files;
  {
    glob_t glob_result;
    glob(ext_pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
      files.insert(glob_result.gl_pathv[i]);
    }
  }

  // Gets all metadata files in the EXTENSION_PATH
  // Loads information from the metadata files and remove the loaded file from
  // the set 'files'
  std::string meta_pattern(extension_path);
  meta_pattern.append("/");
  meta_pattern.append("*");
  meta_pattern.append(kExtensionMetadataSuffix);
  {
    glob_t glob_result;
    glob(meta_pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
      RegisterExtensionsByMeta(glob_result.gl_pathv[i], &files);
    }
  }

  // Load extensions in the remained files of the set 'files'
  if (!meta_only) {
    for (auto it = files.begin(); it != files.end(); ++it) {
      XWalkExtension* ext = new XWalkExtension(*it, this);
      RegisterExtension(ext);
    }
  }
}

bool XWalkExtensionManager::RegisterSymbols(XWalkExtension* extension) {
  std::string name = extension->name();

  if (extension_symbols_.find(name) != extension_symbols_.end()) {
    LOGGER(WARN) << "Ignoring extension with name already registred. '"
                 << name << "'";
    return false;
  }

  XWalkExtension::StringVector entry_points = extension->entry_points();
  for (auto it = entry_points.begin(); it != entry_points.end(); ++it) {
    if (extension_symbols_.find(*it) != extension_symbols_.end()) {
      LOGGER(WARN) << "Ignoring extension with entry_point already registred. '"
                   << (*it) << "'";
      return false;
    }
  }

  for (auto it = entry_points.begin(); it != entry_points.end(); ++it) {
    extension_symbols_.insert(*it);
  }

  extension_symbols_.insert(name);

  return true;
}

void XWalkExtensionManager::RegisterExtension(XWalkExtension* extension) {
  if (!extension->lazy_loading() && !extension->Initialize()) {
    delete extension;
    return;
  }

  if (!RegisterSymbols(extension)) {
    delete extension;
    return;
  }

  extensions_[extension->name()] = extension;
  LOGGER(DEBUG) << extension->name() << " is registered.";
}

void XWalkExtensionManager::RegisterExtensionsByMeta(
    const std::string& meta_path, StringSet* files) {
  std::string extension_path(EXTENSION_PATH);

  std::ifstream metafile(meta_path.c_str());
  if (!metafile.is_open()) {
    LOGGER(ERROR) << "Fail to open the plugin metadata file :" << meta_path;
    return;
  }

  picojson::value metadata;
  metafile >> metadata;
    if (metadata.is<picojson::array>()) {
    auto& plugins = metadata.get<picojson::array>();
    for (auto plugin = plugins.begin(); plugin != plugins.end(); ++plugin) {
      if (!plugin->is<picojson::object>())
        continue;

      std::string name = plugin->get("name").to_str();
      std::string lib = plugin->get("lib").to_str();
      if (!common::utils::StartsWith(lib, "/")) {
        lib = extension_path + "/" + lib;
      }

      std::vector<std::string> entries;
      auto& entry_points_value = plugin->get("entry_points");
      if (entry_points_value.is<picojson::array>()) {
        auto& entry_points = entry_points_value.get<picojson::array>();
        for (auto entry = entry_points.begin(); entry != entry_points.end();
             ++entry) {
          entries.push_back(entry->to_str());
        }
      }
      XWalkExtension* extension = new XWalkExtension(lib, name, entries, this);
      RegisterExtension(extension);
      files->erase(lib);
    }
  } else {
    LOGGER(ERROR) << meta_path << " is not a valid metadata file.";
  }
  metafile.close();
}

// override
void XWalkExtensionManager::GetRuntimeVariable(
    const char* key, char* value, size_t value_len) {
  common::AppDB* db = common::AppDB::GetInstance();
  std::string ret = db->Get(kAppDBRuntimeSection, key);
  strncpy(value, ret.c_str(), value_len);
}


}  // namespace extensions
