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

#include <ewk_chromium.h>

#include "common/command_line.h"
#include "common/logger.h"
#include "common/profiler.h"
#include "runtime/browser/runtime.h"

int main(int argc, char* argv[]) {
  STEP_PROFILE_START("Start -> Launch Completed");
  STEP_PROFILE_START("Start -> OnCreate");
  // Parse commandline.
  common::CommandLine::Init(argc, argv);

  // Default behavior, run as runtime.
  LOGGER(INFO) << "Runtime process has been created.";
  ewk_init();
  char* chromium_arg_options[] = {
    argv[0],
    const_cast<char*>("--no-sandbox"),
    const_cast<char*>("--enable-file-cookies"),
    const_cast<char*>("--allow-file-access-from-files"),
    const_cast<char*>("--allow-universal-access-from-files")
  };
  const int chromium_arg_cnt =
      sizeof(chromium_arg_options) / sizeof(chromium_arg_options[0]);
  ewk_set_arguments(chromium_arg_cnt, chromium_arg_options);

  int ret = 0;
  // Runtime's destructor should be called before ewk_shutdown()
  {
    runtime::Runtime runtime;
    ret = runtime.Exec(argc, argv);
  }
  ewk_shutdown();
  exit(ret);

  return EXIT_SUCCESS;
}
