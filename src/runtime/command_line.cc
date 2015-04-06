// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "runtime/command_line.h"

#include <cstring>
#include "common/file_utils.h"

namespace wrt {

namespace {

const char* kRuntimeName = "wrt";
const char* kOptionPrefix = "--";
const char* kOptionValueSeparator = "=";

static bool IsValidOptionString(const char* argument) {
  if (NULL != argument &&
      strncmp(argument, kOptionPrefix, strlen(kOptionPrefix)) == 0) {
    return true;
  } else {
    return false;
  }
}

}  // namespace

CommandLine* CommandLine::current_process_commandline_ = NULL;

CommandLine::CommandLine(int argc, char* argv[])
    : argc_(argc), argv_(argv) {
  // Append option or push each arg(not option) into arguments_
  for (int i=1; i < argc; ++i) {
    if (IsValidOptionString(argv[i])) {
      AppendOption(argv[i]);
    } else {
      arguments_.push_back(argv[i]);
    }
  }

  // Parse program name and appid from argv_ or arguments_
  if (argc > 0) {
    program_ = utils::BaseName(argv[0]);
    if (program_ == kRuntimeName) {
      if (arguments_.size() > 0) {
        // Suppose that appid is at the first of arguments_
        appid_ = arguments_[0];
      }
    } else {
      appid_ = program_;
    }
  }
}

CommandLine::~CommandLine() {
}

void CommandLine::AppendOption(const char* argument) {
  std::string option_string(argument);
  std::string option_name;
  std::string option_value;

  int value_separator_pos = option_string.find(kOptionValueSeparator,
                                               strlen(kOptionPrefix));
  if (value_separator_pos >= 0) {
    int substr_len = value_separator_pos - strlen(kOptionPrefix);
    option_name = option_string.substr(strlen(kOptionPrefix), substr_len);
    option_value = option_string.substr(value_separator_pos+1);
  } else {
    option_name = option_string.substr(strlen(kOptionPrefix),
                                       value_separator_pos);
  }

  options_[option_name] = option_value;
}

bool CommandLine::HasOptionName(const std::string& option_name) {
  return (options_.find(option_name) != options_.end());
}

std::string CommandLine::GetOptionValue(const std::string& option_name) {
  if (HasOptionName(option_name)) {
    return options_[option_name];
  } else {
    return std::string();
  }
}

// static
void CommandLine::Reset() {
  if (!!current_process_commandline_) {
    delete current_process_commandline_;
    current_process_commandline_ = NULL;
  }
}

// static
void CommandLine::Init(int argc, char* argv[]) {
  if (!current_process_commandline_) {
    current_process_commandline_ = new CommandLine(argc, argv);
  }
}

// static
CommandLine* CommandLine::ForCurrentProcess() {
  return current_process_commandline_;
}

}  // namespace wrt
