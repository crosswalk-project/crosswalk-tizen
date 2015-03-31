// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_RUNTIME_COMMAND_LINE_H_
#define WRT_RUNTIME_COMMAND_LINE_H_

#include <string>
#include <vector>

namespace wrt {

class CommandLine {
 public:
  static void Init(int argc, char* argv[]);
  static CommandLine* ForCurrentProcess();

  const std::string& appid() const { return appid_; }
  const std::string& program() const { return program_; }
  const std::vector<std::string>& argv() const { return argv_; }

 private:
  CommandLine(int argc, char* argv[]);
  virtual ~CommandLine();

  // The singleton CommandLine instance of current process
  static CommandLine* current_process_commandline_;

  std::string appid_;
  std::string program_;
  std::vector<std::string> argv_;
};

} // namespace wrt

#endif // WRT_RUNTIME_COMMAND_LINE_H_
