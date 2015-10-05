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

#ifndef XWALK_COMMON_COMMAND_LINE_H_
#define XWALK_COMMON_COMMAND_LINE_H_

#include <map>
#include <string>
#include <vector>

namespace common {

class CommandLine {
 public:
  // CommandLine only uses long options
  typedef std::map<std::string, std::string> OptionMap;
  // Arguments which except for option strings
  typedef std::vector<std::string> Arguments;

  static void Init(int argc, char* argv[]);
  static CommandLine* ForCurrentProcess();
  static void Reset();

  // Test if options_ has 'option_name'
  bool HasOptionName(const std::string& option_name);
  // Get the option's value
  std::string GetOptionValue(const std::string& option_name);
  // Get command string include options and arguments
  std::string GetCommandString();

  std::string GetAppIdFromCommandLine(const std::string& program);

  std::string program() const { return program_; }
  const OptionMap& options() const { return options_; }
  const Arguments& arguments() const { return arguments_; }
  char** argv() const { return argv_; }
  int argc() const { return argc_; }

 private:
  CommandLine(int argc, char* argv[]);
  virtual ~CommandLine();

  void AppendOption(const char* value);

  // The singleton CommandLine instance of current process
  static CommandLine* current_process_commandline_;

  std::string program_;
  OptionMap options_;
  Arguments arguments_;
  int argc_;
  char** argv_;
};

}  // namespace common

#endif  // XWALK_COMMON_COMMAND_LINE_H_
