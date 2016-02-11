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

#include "runtime/browser/web_view.h"

#include <ewk_chromium.h>
#include <functional>
#include <sstream>

#include "runtime/browser/native_window.h"
#include "runtime/browser/web_view_impl.h"

namespace runtime {

WebView::WebView(NativeWindow* window, Ewk_Context* context)
    : impl_(new WebViewImpl(this, window, context)) {
}

WebView::~WebView() {
  delete impl_;
}

void WebView::LoadUrl(const std::string& url, const std::string& mime) {
  impl_->LoadUrl(url, mime);
}

std::string WebView::GetUrl() {
  return impl_->GetUrl();
}

void WebView::Suspend() {
  impl_->Suspend();
}

void WebView::Resume() {
  impl_->Resume();
}

void WebView::Reload() {
  impl_->Reload();
}

void WebView::Backward() {
  impl_->Backward();
}

void WebView::SetVisibility(bool show) {
  impl_->SetVisibility(show);
}

bool WebView::EvalJavascript(const std::string& script) {
  return impl_->EvalJavascript(script);
}

void WebView::SetEventListener(EventListener* listener) {
  impl_->SetEventListener(listener);
}

Evas_Object* WebView::evas_object() const {
  return impl_->evas_object();
}

void WebView::SetAppInfo(const std::string& app_name,
                         const std::string& version) {
  impl_->SetAppInfo(app_name, version);
}

bool WebView::SetUserAgent(const std::string& user_agent) {
  return impl_->SetUserAgent(user_agent.c_str());
}

void WebView::SetCSPRule(const std::string& rule, bool report_only) {
  impl_->SetCSPRule(rule, report_only);
}

void WebView::SetDefaultEncoding(const std::string& encoding) {
  impl_->SetDefaultEncoding(encoding);
}

}  // namespace runtime
