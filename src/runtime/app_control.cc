// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/app_control.h"

#include <appsvc.h>
#include <algorithm>
#include <memory>

#include "common/logger.h"

namespace wrt {

namespace {
}  // namespace

AppControl::AppControl(app_control_h app_control) {
  app_control_to_bundle(app_control, &app_control_bundle_);
  bundle_encode(app_control_bundle_, &app_control_bundle_raw_,
                &app_control_bundle_raw_len_);
}

AppControl::~AppControl() {
  if (app_control_bundle_ != NULL) {
    bundle_free(app_control_bundle_);
  }
  if (app_control_bundle_raw_ != NULL) {
    bundle_free_encoded_rawdata(&app_control_bundle_raw_);
  }
}

std::string AppControl::operation() const {
  const char* operation = appsvc_get_operation(app_control_bundle_);

  if (operation != NULL) {
    return std::string(operation);
  } else {
    return std::string();
  }
}

std::string AppControl::mime() const {
  const char* mime = appsvc_get_mime(app_control_bundle_);

  if (mime != NULL) {
    return std::string(mime);
  } else {
    return std::string();
  }
}

std::string AppControl::uri() const {
  const char* uri = appsvc_get_uri(app_control_bundle_);

  if (uri != NULL) {
    return std::string(uri);
  } else {
    return std::string();
  }
}

std::string AppControl::category() const {
  const char* category = appsvc_get_category(app_control_bundle_);

  if (category != NULL) {
    return std::string(category);
  } else {
    return std::string();
  }
}

std::string AppControl::data(const std::string& key) const {
  const char* data = appsvc_get_data(app_control_bundle_, key.c_str());

  if (data != NULL) {
    return std::string(data);
  } else {
    return std::string();
  }
}

std::vector<std::string> AppControl::data_array(const std::string& key) const {
  int data_array_len = 0;
  const char** data_array = appsvc_get_data_array(app_control_bundle_,
                                                  key.c_str(), &data_array_len);
  std::vector<std::string> data_vector;

  if (data_array_len > 0) {
    for (int i = 0; i < data_array_len; i++) {
      data_vector.push_back(data_array[i]);
    }
  }
  return data_vector;
}

std::string AppControl::encoded_bundle() const {
  return std::string(reinterpret_cast<char*>(app_control_bundle_raw_),
                     app_control_bundle_raw_len_);
}

bool AppControl::IsDataArray(const std::string& key) {
  return appsvc_data_is_array(app_control_bundle_, key.c_str());
}

bool AppControl::AddData(const std::string& key, const std::string& value) {
  int result = appsvc_add_data(app_control_bundle_, key.c_str(), value.c_str());
  if (result < 0) {
    LoggerE("AppControl::AddData Fail");
    return false;
  } else {
    return true;
  }
}

bool AppControl::AddDataArray(const std::string& key,
                              const std::vector<std::string>& value_array) {
  int n = value_array.size();
  std::vector<const char*> v;
  std::for_each(value_array.begin(), value_array.end(),
                [&v] (std::string str) {
                  v.push_back(static_cast<const char*>(str.c_str()));
                });

  int result = appsvc_add_data_array(app_control_bundle_, key.c_str(),
                                     v.data(), n);
  if (result < 0) {
    LoggerE("AppControl::AddDataArray Fail");
    return false;
  } else {
    return true;
  }
}

}  // namespace wrt
