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

#ifndef WRT_BUNDLE_EXTENSION_RENDERER_CONTROLLER_H_
#define WRT_BUNDLE_EXTENSION_RENDERER_CONTROLLER_H_

#include <v8/v8.h>
#include <string>
#include <memory>

namespace wrt {

class ExtensionClient;

class ExtensionRendererController {
 public:
  static ExtensionRendererController& GetInstance();

  void DidCreateScriptContext(v8::Handle<v8::Context> context);
  void WillReleaseScriptContext(v8::Handle<v8::Context> context);

  bool InitializeExtensions(const std::string& uuid);

 private:
  ExtensionRendererController();
  virtual ~ExtensionRendererController();

 private:
  std::unique_ptr<ExtensionClient> extensions_client_;
};

}  // namespace wrt

#endif  // WRT_BUNDLE_EXTENSION_RENDERER_CONTROLLER_H_
