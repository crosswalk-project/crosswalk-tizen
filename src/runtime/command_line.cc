// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "command_line.h"

#include <string>

#include "file_utils.h"

namespace wrt {

namespace {
  const char* kRuntimeName = "wrt";
}

CommandLine* CommandLine::current_process_commandline_ = NULL;

CommandLine::CommandLine(int argc, char* argv[]) {
  // parse program name and appid from arguments
  if (argc > 0) {
    program_ = utils::BaseName(argv[0]);
    if (program_ == kRuntimeName) {
      if (argc > 1) {
        appid_ = argv[1];
      }
    } else {
      appid_ = program_;
    }
  }

  // make a string vector with arguments
  for (int i=0; i < argc; ++i) {
    argv_.push_back(argv[i]);
  }
}

CommandLine::~CommandLine() {
}

void CommandLine::Init(int argc, char* argv[]) {
  if (!current_process_commandline_) {
    current_process_commandline_ = new CommandLine(argc, argv);
  }
}

CommandLine* CommandLine::ForCurrentProcess() {
  return current_process_commandline_;
}



} // namespace wrt
