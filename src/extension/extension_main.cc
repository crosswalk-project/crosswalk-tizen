// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <glib.h>
#include <glib-unix.h>

#include "common/logger.h"
#include "common/command_line.h"
#include "extension/extension_server.h"

namespace {

gboolean HandleQuitSignal(gpointer data) {
  GMainLoop* loop = reinterpret_cast<GMainLoop*>(data);
  g_main_loop_quit(loop);
  return false;
}

}  // namespace

// TODO(wy80.choi): This main function should be merged to wrt runtime
// to reduce static code size of memory.
int main(int argc, char* argv[]) {
  GMainLoop* loop;

  loop = g_main_loop_new(NULL, FALSE);
  g_unix_signal_add(SIGINT, HandleQuitSignal, loop);
  g_unix_signal_add(SIGTERM, HandleQuitSignal, loop);

  wrt::CommandLine::Init(argc, argv);
  wrt::CommandLine* cmd = wrt::CommandLine::ForCurrentProcess();

  // TODO(wy80.choi): Receive extension paths for user defined extensions.

  // Receive AppID from arguments.
  if (cmd->arguments().size() < 1) {
    LoggerE("uuid is required.");
    exit(1);
  }
  std::string uuid = cmd->arguments()[0];

  // Start ExtensionServer
  wrt::ExtensionServer server(uuid);
  if (!server.Start()) {
    LoggerE("Failed to start extension server.");
    exit(1);
  }

  LoggerI("extension process has been started.");

  g_main_loop_run(loop);

  LoggerI("extension process is exiting.");

  g_main_loop_unref(loop);

  return 0;
}
