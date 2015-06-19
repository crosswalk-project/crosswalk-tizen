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

#ifndef WRT_BUNDLE_OBJECT_TOOLS_MODULE_H_
#define WRT_BUNDLE_OBJECT_TOOLS_MODULE_H_

#include "bundle/module_system.h"

namespace wrt {

class ObjectToolsModule : public NativeModule {
 public:
  ObjectToolsModule();
  ~ObjectToolsModule() override;

 private:
  v8::Handle<v8::Object> NewInstance() override;
  v8::Persistent<v8::Function> create_function_;
};

}  // namespace wrt

#endif  // WRT_BUNDLE_OBJECT_TOOLS_MODULE_H_

