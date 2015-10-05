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

#include "common/url.h"

#include <algorithm>
#include <stdexcept>

#include "common/logger.h"
#include "common/string_utils.h"

namespace common {

namespace {

const char* kSchemeTypeFile = "file";
const char* kSchemeTypeHttp = "http";
const char* kSchemeTypeHttps = "https";
const char* kSchemeTypeSsh = "ssh";
const char* kSchemeTypeFtp = "ftp";

// length of scheme identifier ://
const int kSchemeIdLen = 3;
int kPortHttp = 80;
int kPortHttps = 443;
int kPortSsh = 22;
int kPortFtp = 21;
int kPortDefault = 0;

int GetDefaultPort(const std::string& scheme) {
  if (scheme == kSchemeTypeHttp) return kPortHttp;
  else if (scheme == kSchemeTypeHttps) return kPortHttps;
  else if (scheme == kSchemeTypeSsh) return kPortSsh;
  else if (scheme == kSchemeTypeFtp) return kPortFtp;
  else
    return kPortDefault;
}

}  // namespace

class URLImpl {
 public:
  explicit URLImpl(const std::string& url);
  URLImpl() {}

  std::string url() const { return url_; }
  std::string scheme() const { return scheme_; }
  std::string domain() const { return domain_; }
  int port() const { return port_; }
  std::string path() const { return path_; }

 private:
  std::string url_;
  std::string scheme_;
  std::string domain_;
  int port_;
  std::string path_;

  bool ExtractScheme();
  void ExtractDomain();
  void ExtractDomainPort();
  void ExtractPath();
};

URLImpl::URLImpl(const std::string& url) : port_(0) {
  if (url.empty())
    return;

  url_ = url;
  if (!ExtractScheme()) {
    ExtractDomain();
    ExtractPath();
    return;
  }

  if (scheme_ != kSchemeTypeFile)
    ExtractDomainPort();

  ExtractPath();
}

bool URLImpl::ExtractScheme() {
  size_t end_of_scheme = 0;
  if (url_.find("://") != std::string::npos) {
    end_of_scheme = url_.find("://");
    std::string scheme = url_.substr(0, end_of_scheme);
    std::transform(scheme.begin(), scheme.end(), scheme.begin(), ::tolower);
    scheme_ = scheme;
    return true;
  } else {
    return false;
  }
}

void URLImpl::ExtractDomain() {
  size_t start_of_domain = scheme_.empty() ?
                           0 : scheme_.length() + kSchemeIdLen;
  size_t end_of_domain = url_.find_first_of('/', start_of_domain);
  domain_ =
    url_.substr(start_of_domain, end_of_domain == std::string::npos ?
                std::string::npos : end_of_domain - start_of_domain);
}

void URLImpl::ExtractDomainPort() {
  ExtractDomain();
  std::string domain = domain_;

  // Decide start position to find port considering IPv6 case
  size_t start_pos = domain.find('@');
  start_pos = (start_pos != std::string::npos) ? start_pos + 1 : 0;
  if (domain[start_pos] == '[')
    start_pos = domain.find(']', start_pos+1);

  size_t port_separator =
    domain.find_first_of(':', start_pos != std::string::npos ? start_pos : 0);
  if (port_separator != std::string::npos) {
    domain_ = domain.substr(0, port_separator);
    std::string port = domain.substr(port_separator+1);
    if (port.empty()) {
      port_ = GetDefaultPort(scheme_);
    } else {
      try {
        port_ = std::stoi(port);
      } catch (...) {
        port_ = GetDefaultPort(scheme_);
      }
    }
  } else {
    domain_ = domain;
    port_ = GetDefaultPort(scheme_);
  }
}

void URLImpl::ExtractPath() {
  std::string sub_url = url_.substr(scheme_.empty() ?
                                    0 : scheme_.length() + kSchemeIdLen);
  if (domain_.empty()) {
    path_ = sub_url;
  } else {
    size_t start_of_path = sub_url.find_first_of('/');
    if (start_of_path != std::string::npos)
      path_ = sub_url.substr(start_of_path);
  }
}

URL::URL(const std::string& url) {
  impl_ = new URLImpl(url);
}

std::string URL::url() const { return impl_->url(); }
std::string URL::scheme() const { return impl_->scheme(); }
std::string URL::domain() const { return impl_->domain(); }
int URL::port() const { return impl_->port(); }
std::string URL::path() const { return impl_->path(); }

URL::~URL() {
  delete impl_;
}

}  // namespace common
