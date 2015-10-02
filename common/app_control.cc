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

#include "common/app_control.h"

#include <appsvc.h>
#include <app_control_internal.h>

#include <algorithm>
#include <memory>
#include <map>
#include <regex> // NOLINT
#include <utility>
#include <vector>

#include "common/file_utils.h"
#include "common/logger.h"
#include "common/string_utils.h"

namespace common {

namespace {
static bool BundleAddData(bundle* target, const std::string& key,
                          const std::string& value) {
  int result = appsvc_add_data(target, key.c_str(), value.c_str());
  if (result < 0) {
    LOGGER(ERROR) << "Failed to add data to appsvc.";
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
    LOGGER(ERROR) << "Failed to add an array of data to appsvc.";
    return false;
  } else {
    return true;
  }
}

static const std::string GetOperationFromScheme(const std::string& scheme) {
  static std::map<const std::string, const std::string> table = {
    {"sms", APP_CONTROL_OPERATION_COMPOSE},
    {"mmsto", APP_CONTROL_OPERATION_COMPOSE},
    {"mailto", APP_CONTROL_OPERATION_COMPOSE},
    {"tel", APP_CONTROL_OPERATION_CALL}
  };
  auto found = table.find(scheme);
  if (found == table.end()) {
    // default operation
    return APP_CONTROL_OPERATION_VIEW;
  }
  return found->second;
}

static void AppendExtraDatafromUrl(AppControl* request,
                                   const std::string& url) {
  static std::vector<std::pair<std::string, std::string> > patterns = {
    {".*[?&]body=([^&]+).*", APP_CONTROL_DATA_TEXT},
    {".*[?&]cc=([^&]+).*", APP_CONTROL_DATA_CC},
    {".*[?&]bcc=([^&]+).*", APP_CONTROL_DATA_BCC},
    {".*[?&]subject=([^&]+).*", APP_CONTROL_DATA_SUBJECT},
    {".*[?&]to=([^&]+).*", APP_CONTROL_DATA_TO},
    {"sms:([^?&]+).*", APP_CONTROL_DATA_TO},
    {"mmsto:([^?&]+).*", APP_CONTROL_DATA_TO},
    {"mailto:([^?&]+).*", APP_CONTROL_DATA_TO}
  };

  for (auto& param : patterns) {
    std::regex pattern(param.first, std::regex_constants::icase);
    std::smatch result;
    if (std::regex_match(url, result, pattern) && result.size() >= 2) {
      std::string extra_data = result[1].str();
      request->AddData(
        param.second,
        utils::UrlDecode(extra_data));
    }
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
    LOGGER(ERROR) << "Failed to craete result bundle.";
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

std::unique_ptr<AppControl> AppControl::MakeAppcontrolFromURL(
    const std::string& url) {
  std::string smsto_scheme("smsto");

  std::string request_url(url);
  std::string scheme = utils::SchemeName(request_url);
  // smsto: does not supported by platform. change to sms:
  if (scheme == smsto_scheme) {
    request_url = "sms" + request_url.substr(smsto_scheme.length());
    scheme = "sms";
  }

  std::unique_ptr<AppControl> request(new AppControl());
  request->set_uri(request_url);
  request->set_operation(GetOperationFromScheme(scheme));
  AppendExtraDatafromUrl(request.get(), request_url);

  return std::move(request);
}

}  // namespace common
