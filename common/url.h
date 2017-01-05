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

#ifndef XWALK_COMMON_URL_H_
#define XWALK_COMMON_URL_H_

#include <string>

namespace common {

class URLImpl;

/*
 * This class parses a given url based on its scheme type.
 * The parsed data is stored in different variables depending on the scheme
 * type.
 * The following shows the variables which are used for each scheme type:
 * http:// https:// ssh:// ftp://
 *   => url_, scheme_, domain_, port_, path_
 * app://
 *   => url_, scheme_, domain_, path_
 * file://
 *   => url_, scheme_, path_
 * No Scheme Type
 *   => domain_, path_
 *
 * If the url does not have specific data, an empty string will be stored
 * in the corresponding variables.(RFC 1738)
 *
 * ex) http://user:password@www.tizen.org:8080/market/Item?12345
 * url_ = http://user:password@www.tizen.org:8080/market/Item?12345
 * scheme_ = http
 * user_ = user
 * password_ = password
 * domain_ = www.tizen.org
 * port_ = 8080
 * path_ = /market/Item?12345
*/
class URL {
 public:
  explicit URL(const std::string& url);
  ~URL();

  std::string url() const;
  std::string scheme() const;
  std::string domain() const;
  int port() const;
  std::string path() const;

 private:
  URLImpl* impl_;
};

}  // namespace common

#endif  // XWALK_COMMON_URL_H_
