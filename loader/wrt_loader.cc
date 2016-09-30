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
#include <dlfcn.h>
#include <dlog.h>

// loader file must have "User" execute label, because launchpad daemon runs 
// with "System::Privileged" label.
int main(int argc, char* argv[]) {
  void* handle = dlopen("/usr/bin/xwalk_runtime", RTLD_NOW);
  if (!handle) {
    dlog_print(DLOG_DEBUG, "XWALK", "Error loading xwalk_runtime");
    return false;
  }

  typedef int (*MAIN_FUNC)(int argc, char* argv[]);

  MAIN_FUNC real_main = reinterpret_cast<MAIN_FUNC>(dlsym(handle, "main"));
  if (!real_main) {
    dlog_print(DLOG_DEBUG, "XWALK", "Error loading real_main");
    return false;      
  }

  int ret = real_main(argc, argv);
  dlclose(handle);

  return ret;
}
