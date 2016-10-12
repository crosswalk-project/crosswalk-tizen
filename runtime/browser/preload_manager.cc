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

#include "runtime/browser/preload_manager.h"

#include <Elementary.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <stdio.h>

#include "common/logger.h"
#include "runtime/browser/native_app_window.h"
#include "extensions/common/xwalk_extension_server.h"

#ifndef INJECTED_BUNDLE_PATH
#error INJECTED_BUNDLE_PATH is not set.
#endif

namespace runtime {

PreloadManager* PreloadManager::GetInstance() {
  static PreloadManager instance;
  return &instance;
}

PreloadManager::PreloadManager() {
}

void PreloadManager::CreateCacheComponet() {
  // Load extensions in the preload step
  auto extension_server = extensions::XWalkExtensionServer::GetInstance();
  extension_server->Preload();

  // It was not used. just to fork zygote process
  auto context =
      ewk_context_new_with_injected_bundle_path(INJECTED_BUNDLE_PATH);
  context = context;  // To prevent build warning
  cached_window_.reset(new NativeAppWindow());
  cached_window_->Initialize();
}

NativeWindow* PreloadManager::GetCachedNativeWindow() {
  return cached_window_ ? cached_window_.release() : nullptr;
}

void PreloadManager::ReleaseCachedNativeWindow() {
  cached_window_.reset(nullptr);
}

}  //  namespace runtime
