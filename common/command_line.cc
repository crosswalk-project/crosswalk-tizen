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

#include "common/command_line.h"

#include <cstring>

#include "common/file_utils.h"

namespace common {

namespace {

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
  program_ = std::string(argv[0]);
}

CommandLine::~CommandLine() {
}

void CommandLine::AppendOption(const char* value) {
  std::string option_string(value);
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

std::string CommandLine::GetCommandString() {
  std::string result;
  result.append(program_);
  result.append(" ");
  for (auto& it : options_) {
    result.append(kOptionPrefix);
    result.append(it.first);
    if (!it.second.empty()) {
      result.append(kOptionValueSeparator);
      result.append(it.second);
    }
    result.append(" ");
  }
  for (auto& it : arguments_) {
    result.append(it);
    result.append(" ");
  }
  return result;
}

std::string CommandLine::GetAppIdFromCommandLine(const std::string& program) {
  if (argc_ > 0) {
    std::string tmp = utils::BaseName(program_);
    if (tmp == program) {
      if (arguments_.size() > 0) {
        // Suppose that appid is at the first of arguments_
        return arguments_[0];
      }
    } else {
      return tmp;
    }
  }
  return std::string();
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

}  // namespace common
