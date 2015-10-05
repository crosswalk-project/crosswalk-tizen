// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <glib.h>
#include <glib-unix.h>

#include "common/command_line.h"
#include "common/logger.h"
#include "extensions/extension/xwalk_extension_server.h"

int main(int argc, char* argv[]) {
  GMainLoop* loop;

  loop = g_main_loop_new(NULL, FALSE);

  // Register Quit Signal Handlers
  auto quit_callback = [](gpointer data) -> gboolean {
    GMainLoop* loop = reinterpret_cast<GMainLoop*>(data);
    g_main_loop_quit(loop);
    return false;
  };
  g_unix_signal_add(SIGINT, quit_callback, loop);
  g_unix_signal_add(SIGTERM, quit_callback, loop);

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

  g_main_loop_run(loop);

  LOGGER(INFO) << "extension process is exiting.";

  g_main_loop_unref(loop);

  return true;
}
