// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Ecore.h>

#include "common/command_line.h"
#include "common/logger.h"
#include "extensions/extension/xwalk_extension_server.h"

Ecore_Event_Handler* quit_handler = NULL;

int main(int argc, char* argv[]) {
  ecore_init();

  // Register Quit Signal Handlers
  auto quit_callback = [](void*, int, void*) -> Eina_Bool {
    ecore_main_loop_quit();
    return EINA_TRUE;
  };
  ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT,
                          quit_callback, NULL);

  common::CommandLine::Init(argc, argv);
  common::CommandLine* cmd = common::CommandLine::ForCurrentProcess();

  // TODO(wy80.choi): Receive extension paths for user defined extensions.

  // Receive appid from arguments.
  if (cmd->arguments().size() < 1) {
    LOGGER(ERROR) << "appid is required.";
    return false;
  }
  std::string appid = cmd->arguments()[0];

  // Start ExtensionServer
  extensions::XWalkExtensionServer server(appid);
  if (!server.Start()) {
    LOGGER(ERROR) << "Failed to start extension server.";
    return false;
  }

  LOGGER(INFO) << "extension process has been started.";
  ecore_main_loop_glib_integrate();
  ecore_main_loop_begin();

  LOGGER(INFO) << "extension process is exiting.";
  ecore_shutdown();

  return true;
}
