// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/web_view.h"

#include <ewk_chromium.h>
#include <functional>
#include <sstream>

#include "runtime/native_window.h"
#include "runtime/web_view_impl.h"

namespace wrt {

WebView::WebView(wrt::NativeWindow* window, Ewk_Context* context)
    : impl_(new WebViewImpl(this, window, context)) {
}

WebView::~WebView() {
  delete impl_;
}

void WebView::LoadUrl(const std::string& url) {
  impl_->LoadUrl(url);
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

void WebView::SetUserAgent(const std::string& user_agent) {
  impl_->SetUserAgent(user_agent.c_str());
}
}  // namespace wrt
