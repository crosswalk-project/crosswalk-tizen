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

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_H_

#include <app.h>
#include <memory>
#include <string>

#include "common/application_data.h"
#include "runtime/browser/web_application.h"

namespace runtime {

class Runtime {
 public:
  virtual ~Runtime() = 0;

  virtual int Exec(int argc, char* argv[]) = 0;

  static std::unique_ptr<Runtime> MakeRuntime(
    common::ApplicationData* app_data);

 protected:
  void ClosePageFromOnTerminate(WebApplication* app);
};

}  // namespace runtime

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_H_
