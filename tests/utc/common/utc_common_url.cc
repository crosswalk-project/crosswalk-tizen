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

#include <limits.h>
#include "common/url.h"
#include "gtest/gtest.h"
#include <string>
#include <iostream>

namespace wrt {

int GetDefaultPort(const std::string& scheme) {
 if (scheme == "http")
   return 80;
 else if (scheme == "https")
   return 443;
 else if (scheme == "ssh")
   return 22;
 else if (scheme == "ftp")
   return 21;
 else
   return 0;
}

void TestExpectEq(const std::string& test_url, const std::string& scheme,
                 const std::string& domain, int port, const std::string path) {
  URL url(test_url);
  EXPECT_EQ(scheme, url.scheme());
  EXPECT_EQ(port, url.port());
  EXPECT_EQ(domain, url.domain());
  EXPECT_EQ(path, url.path());
}

// Tests URL Class

TEST(SchemeHttpTest, Positive) {
  // "scheme://domain:port/path"
  std::string scheme;
  std::string domain;
  int port;
  std::string path;
  std::string url;

  // normal
  url = "https://username:password@samsung.com:443/dir1/abc.jpg";
  scheme = "https";
  domain = "username:password@samsung.com";
  port = 443;
  path = "/dir1/abc.jpg";
  TestExpectEq(url, scheme, domain, port, path);

  // no port
  url = "https://username:password@samsung.com/dir1/abc.jpg";
  scheme = "https";
  domain = "username:password@samsung.com";
  port = GetDefaultPort("https");
  path = "/dir1/abc.jpg";
  TestExpectEq(url, scheme, domain, port, path);

  // no path
  url = "https://username:password@samsung.com:443";
  scheme = "https";
  domain = "username:password@samsung.com";
  port = 443;
  path = "";
  TestExpectEq(url, scheme, domain, port, path);

  // no path,port
  url = "https://username:password@samsung.com";
  scheme = "https";
  domain = "username:password@samsung.com";
  port = GetDefaultPort("https");
  path = "";
  TestExpectEq(url, scheme, domain, port, path);

  // ugly scheme
  url = "HtTp://username:password@samsung.com/path";
  scheme = "http";
  domain = "username:password@samsung.com";
  port = GetDefaultPort("http");
  path = "/path";
  TestExpectEq(url, scheme, domain, port, path);

  // ugly scheme with no userinfo
  url = "HtTp://www.samsung.com/path";
  scheme = "http";
  domain = "www.samsung.com";
  port = GetDefaultPort("http");
  path = "/path";
  TestExpectEq(url, scheme, domain, port, path);

  // normal with port 8080
  url = "http://www.naver.com:8080/";
  scheme = "http";
  domain = "www.naver.com";
  port = 8080;
  path = "/";
  TestExpectEq(url, scheme, domain, port, path);

  // normal with port 80
  url = "http://www.naver.com:80/";
  scheme = "http";
  domain = "www.naver.com";
  port = 80;
  path = "/";
  TestExpectEq(url, scheme, domain, port, path);
}

TEST(SchemeHttpTest, Negative) {
  // "scheme://domain:port/path"
  std::string scheme;
  std::string domain;
  int port = 0;
  std::string path;
  std::string url;

  // no url
  url = "";
  scheme = "";
  domain = "";
  port = 0;
  path = "";
  TestExpectEq(url, scheme, domain, port, path);

  // no scheme
  url = "username:password@samsung.com:80/dir1/path";
  scheme = "";
  domain = "username:password@samsung.com:80";
  port = 0;
  path = "/dir1/path";
  TestExpectEq(url, scheme, domain, port, path);

  // no domain
  url = "https://";
  scheme = "https";
  domain = "";
  port = GetDefaultPort("https");;
  path = "";
  TestExpectEq(url, scheme, domain, port, path);

  // invalid port
  url = "ftp://ftp.wordpress.com:invalid/path";
  scheme = "ftp";
  domain = "ftp.wordpress.com";
  port = GetDefaultPort("ftp");;
  path = "/path";
  TestExpectEq(url, scheme, domain, port, path);
}

TEST(SchemeAppTest, Positive) {
  std::string scheme;
  std::string domain;
  int port = 0;
  std::string path;
  std::string url;

  //app://appid
  url = "app://appid";
  scheme = "app";
  domain = "appid";
  path = "";
  TestExpectEq(url, scheme, domain, port, path);

  //app://appid/path
  url = "app://appid/path/path2";
  scheme = "app";
  domain = "appid";
  path = "/path/path2";
  TestExpectEq(url, scheme, domain, port, path);
}

TEST(SchemeFileTest, Positive) {
  std::string url;
  std::string scheme;
  std::string domain;
  int port = 0;
  std::string path;

  // normal file scheme
  url = "file://file/scheme/path";
  scheme = "file";
  path = "file/scheme/path";
  TestExpectEq(url, scheme, domain, port, path);

  // ugly file scheme
  url = "fiLE://///path/";
  scheme = "file";
  path = "///path/";
  TestExpectEq(url, scheme, domain, port, path);
}

}  // namespace wrt
