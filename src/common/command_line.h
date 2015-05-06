// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_COMMAND_LINE_H_
#define WRT_COMMON_COMMAND_LINE_H_

#include <string>
#include <map>
#include <vector>

namespace wrt {

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

  std::string appid() const { return appid_; }
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

  std::string appid_;
  std::string program_;
  OptionMap options_;
  Arguments arguments_;
  int argc_;
  char** argv_;
};

}  // namespace wrt

#endif  // WRT_COMMON_COMMAND_LINE_H_
