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

#include "runtime/browser/prelauncher.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <launchpad.h>

#include <csignal>
#include <memory>

#include "common/logger.h"
#include "common/profiler.h"

namespace runtime {

/* Already changed the privilege from the launchpad.
namespace {
std::string g_smacklabel;

int SmackLabelSetForTask(const std::string& label) {
  int ret;
  int fd;
  char path[1024] = {0};
  int tid = static_cast<int>(syscall(__NR_gettid));
  snprintf(path, sizeof(path), "/proc/%d/attr/current", tid);
  fd = open(path, O_WRONLY);
  if (fd < 0)
    return -1;
  ret = write(fd, label.c_str(), label.length());
  if (syncfs(fd) < 0) {
    close(fd);
    return -1;
  }
  close(fd);
  return (ret < 0) ? -1 : 0;
}

void ChangePrivilegeForThreads(const std::string& appid) {
  SCOPE_PROFILE();
  g_smacklabel = "User::App::" + appid;
  auto oldhandler = std::signal(SIGUSR1, [](int signo){
    SmackLabelSetForTask(g_smacklabel);
  });

  int current_tid = static_cast<int>(syscall(__NR_gettid));

  DIR* dir;
  struct dirent entry;
  struct dirent* result;

  if ((dir = opendir("/proc/self/task")) != NULL) {
    while (readdir_r(dir, &entry, &result) == 0 && result != NULL) {
      if (strcmp(entry.d_name, ".") == 0 || strcmp(entry.d_name, "..") == 0)
        continue;
      int tid = atoi(entry.d_name);
      if (tid == current_tid)
        continue;
      syscall(__NR_tkill, tid, SIGUSR1);
    }
    closedir(dir);
  }
  signal(SIGUSR1, oldhandler);
}

}  // namespace
*/

PreLauncher::PreLauncher() {
  ecore_init();
}

PreLauncher::~PreLauncher() {
  ecore_shutdown();
}

void PreLauncher::StartMainLoop() {
  ecore_main_loop_begin();
}

void PreLauncher::StopMainLoop() {
  ecore_main_loop_quit();
}

void PreLauncher::Watch(int fd, std::function<void(int)> readable) {
  auto callback = [](void *user_data, Ecore_Fd_Handler *fd_handler) {
    PreLauncher* launcher = static_cast<PreLauncher*>(user_data);
    if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ)) {
      int fd = ecore_main_fd_handler_fd_get(fd_handler);
      auto handler = launcher->handlers_[fd_handler];
      if (handler)
        handler(fd);
    }
    return ECORE_CALLBACK_RENEW;
  };
  Ecore_Fd_Handler* handler =
      ecore_main_fd_handler_add(fd,
          static_cast<Ecore_Fd_Handler_Flags>(ECORE_FD_READ),
          callback, this, NULL, NULL);
  handlers_[handler] = readable;
  fd_map_[fd] = handler;
}
void PreLauncher::Unwatch(int fd) {
  auto found = fd_map_.find(fd);
  if (found == fd_map_.end())
    return;
  auto handle = found->second;
  ecore_main_fd_handler_del(handle);
  fd_map_.erase(found);
  handlers_.erase(handle);
}


int PreLauncher::Prelaunch(int argc, char* argv[],
    Preload preload, DidStart didstart, RealMain realmain) {
  std::unique_ptr<PreLauncher> launcher(new PreLauncher());
  launcher->preload_ = preload;
  launcher->didstart_ = didstart;
  launcher->realmain_ = realmain;

  auto create = [](bundle* extra, int type, void *user_data) {
    PreLauncher* launcher = static_cast<PreLauncher*>(user_data);
    launcher->preload_();
  };

  auto launch = [](int argc, char **argv, const char *app_path,
                   const char *appid, const char *pkgid,
                   const char *pkg_type, void *user_data) {
    PreLauncher* launcher = static_cast<PreLauncher*>(user_data);
    /* Already changed the privilege from the launchpad. */
    //ChangePrivilegeForThreads(appid);
    launcher->didstart_(app_path);
    return 0;
  };

  auto terminate = [](int argc, char **argv, void *user_data) {
    PreLauncher* launcher = static_cast<PreLauncher*>(user_data);
    return launcher->realmain_(argc, argv);
  };

  auto start_loop = [](void *user_data) {
    PreLauncher* launcher = static_cast<PreLauncher*>(user_data);
    launcher->StartMainLoop();
  };
  auto stop_loop = [](void *user_data) {
    PreLauncher* launcher = static_cast<PreLauncher*>(user_data);
    launcher->StopMainLoop();
  };
  auto add_fd = [](void *user_data, int fd, loader_receiver_cb receiver) {
    PreLauncher* launcher = static_cast<PreLauncher*>(user_data);
    launcher->Watch(fd, receiver);
  };
  auto remove_fd = [](void *user_data, int fd) {
    PreLauncher* launcher = static_cast<PreLauncher*>(user_data);
    launcher->Unwatch(fd);
  };

  loader_lifecycle_callback_s callbacks = {
    create,
    launch,
    terminate
  };

  loader_adapter_s loop_methods = {
    start_loop,
    stop_loop,
    add_fd,
    remove_fd
  };

  return launchpad_loader_main(argc, argv,
                               &callbacks,
                               &loop_methods,
                               launcher.get());
}

}  // namespace runtime
