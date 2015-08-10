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

#include "common/logger.h"
#include "common/constants.h"
#include "common/command_line.h"
#include "runtime/runtime.h"
#include "extension/extension_server.h"

int main(int argc, char* argv[]) {
  // Parse commandline.
  wrt::CommandLine::Init(argc, argv);

  wrt::CommandLine* cmd = wrt::CommandLine::ForCurrentProcess();
  if (cmd->HasOptionName(wrt::kSwitchExtensionServer)) {
    // If cmd has the switch '--extension-server', run as extension server.
    LOGGER(INFO) << "Extension server process has been created.";
    if (!wrt::ExtensionServer::StartExtensionProcess()) {
      LOGGER(ERROR) << "Failed to start extension server.";
      exit(EXIT_FAILURE);
    }
  } else {
    // Default behavior, run as runtime.
    LOGGER(INFO) << "Runtime process has been created.";
    ewk_init();
    char* chromium_arg_options[] = {
      argv[0],
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
      wrt::Runtime runtime;
      ret = runtime.Exec(argc, argv);
    }
    ewk_shutdown();
    exit(ret);
  }

  return EXIT_SUCCESS;
}
