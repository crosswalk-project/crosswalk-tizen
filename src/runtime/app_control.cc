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
static bool BundleAddData(bundle* target, const std::string& key,
                          const std::string& value) {
  int result = appsvc_add_data(target, key.c_str(), value.c_str());
  if (result < 0) {
    LoggerE("BundleAddData : appsvc_add_data fail");
    return false;
  } else {
    return true;
  }
}

static bool BundleAddDataArray(bundle* target, const std::string& key,
                               const std::vector<std::string>& value_array) {
  int n = value_array.size();
  std::vector<const char*> v;
  std::for_each(value_array.begin(), value_array.end(),
                [&v] (std::string str) {
                  v.push_back(static_cast<const char*>(str.c_str()));
                });

  int result = appsvc_add_data_array(target, key.c_str(),
                                     v.data(), n);
  if (result < 0) {
    LoggerE("BundleAddDataArray appsvc_add_data_array fail");
    return false;
  } else {
    return true;
  }
}


}  // namespace

AppControl::AppControl(app_control_h app_control) {
  app_control_clone(&app_control_, app_control);
  app_control_to_bundle(app_control_, &app_control_bundle_);
}

AppControl:: AppControl() {
  app_control_create(&app_control_);
  app_control_to_bundle(app_control_, &app_control_bundle_);
}

AppControl::~AppControl() {
  if (app_control_ != NULL) {
    app_control_destroy(app_control_);
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

void AppControl::set_operation(const std::string& operation) {
  appsvc_set_operation(app_control_bundle_, operation.c_str());
}

std::string AppControl::mime() const {
  const char* mime = appsvc_get_mime(app_control_bundle_);

  if (mime != NULL) {
    return std::string(mime);
  } else {
    return std::string();
  }
}

void AppControl::set_mime(const std::string& mime) {
  appsvc_set_mime(app_control_bundle_, mime.c_str());
}

std::string AppControl::uri() const {
  const char* uri = appsvc_get_uri(app_control_bundle_);

  if (uri != NULL) {
    return std::string(uri);
  } else {
    return std::string();
  }
}

void AppControl::set_uri(const std::string& uri) {
  appsvc_set_uri(app_control_bundle_, uri.c_str());
}

std::string AppControl::category() const {
  const char* category = appsvc_get_category(app_control_bundle_);

  if (category != NULL) {
    return std::string(category);
  } else {
    return std::string();
  }
}

void AppControl::set_category(const std::string& category) {
  appsvc_set_category(app_control_bundle_, category.c_str());
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

std::string AppControl::encoded_bundle() {
  bundle_raw* encoded_data;
  int len;
  bundle_encode(app_control_bundle_, &encoded_data, &len);
  std::unique_ptr<bundle_raw*, decltype(bundle_free_encoded_rawdata)*>
    ptr { &encoded_data, bundle_free_encoded_rawdata};
  return std::string(reinterpret_cast<char*>(encoded_data), len);
}

bool AppControl::IsDataArray(const std::string& key) {
  return appsvc_data_is_array(app_control_bundle_, key.c_str());
}

bool AppControl::AddData(const std::string& key, const std::string& value) {
  return BundleAddData(app_control_bundle_, key, value);
}



bool AppControl::AddDataArray(const std::string& key,
                              const std::vector<std::string>& value_array) {
  return BundleAddDataArray(app_control_bundle_, key, value_array);
}


bool AppControl::Reply(const std::map<std::string,
                                      std::vector<std::string>>& data) {
  bundle* result;
  if (appsvc_create_result_bundle(app_control_bundle_,
                                  &result) != APPSVC_RET_OK) {
    LoggerE("AppControl::Reply Fail : fail to create result bundle");
    return false;
  }
  auto it = data.begin();
  for ( ; it != data.end(); ++it) {
    const std::string& key = it->first;
    if (it->second.size() == 1) {
      BundleAddData(result, key, it->second[0]);
    } else {
      BundleAddDataArray(result, key, it->second);
    }
  }

  int ret = appsvc_send_result(result, APPSVC_RES_OK);
  bundle_free(result);

  return ret == APPSVC_RET_OK ? true : false;
}

bool AppControl::LaunchRequest() {
  return (app_control_send_launch_request(app_control_, NULL, NULL) ==
          APP_CONTROL_ERROR_NONE);
}

}  // namespace wrt
