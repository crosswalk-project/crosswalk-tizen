// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/web_view.h"

#include <functional>

#include "runtime/native_window.h"

namespace wrt {

WebView::WebView(NativeWindow* window, Ewk_Context* context)
    : window_(window),
      context_(context),
      always_run_(false) {
  rotation_handler_id_ = window->AddRotationHandler(
                                    std::bind(&WebView::OnRotation,
                                    this,
                                    std::placeholders::_1));
}

WebView::~WebView() {
  window_->RemoveRotationHandler(rotation_handler_id_);
}

void WebView::LoadUrl(const std::string& url) {
  // TODO(sngn.lee): To be implemented
}

void WebView::Suspend() {
  if (!always_run_) {
    // suspend webview
  }
  // change the visibility
}

void WebView::Resume() {
  if (!always_run_) {
    // resume webview
  }
  // change the visiblity
}

void WebView::Reload() {
}

void WebView::AlwaysRun(bool run) {
  always_run_ = run;
}

bool WebView::EvalJavascript(const std::string& script) {
  return false;
}

void WebView::Initialize() {
}

std::string WebView::GetUrl() {
  return std::string();
}

Evas_Object* WebView::evas_object() const {
  // TODO(sngn.lee): To be implemented
  return NULL;
}

void WebView::OnRotation(int degree) {
  // TOOD(sngn.lee): To be impelmented
}

}  // namespace wrt

