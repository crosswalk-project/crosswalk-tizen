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

#include <ewk_chromium.h>

#include <Elementary.h>

#ifdef WATCH_FACE_FEATURE_SUPPORT
#include <bundle_internal.h>
#include <Ecore_Wayland.h>
#endif  // WATCH_FACE_FEATURE_SUPPORT

#include "common/application_data.h"
#include "common/command_line.h"
#include "common/logger.h"
#include "common/profiler.h"
#include "runtime/browser/runtime.h"
#include "runtime/common/constants.h"
#include "runtime/browser/prelauncher.h"
#include "runtime/browser/preload_manager.h"

bool g_prelaunch = false;

#ifdef WATCH_FACE_FEATURE_SUPPORT
static int setWatchEnv(int argc, char **argv) {
  bundle *kb = NULL;
  char *wayland_display = NULL;
  char *xdg_runtime_dir = NULL;
  char *width_str = NULL;
  char *height_str = NULL;

  if (argc <= 0 || argv == NULL) {
    errno = EINVAL;
    LOGGER(ERROR) << "argument are invalid";
    return -1;
  }

  kb = bundle_import_from_argv(argc, argv);
  if (kb) {
    bundle_get_str(kb, "XDG_RUNTIME_DIR", &xdg_runtime_dir);
    bundle_get_str(kb, "WAYLAND_DISPLAY", &wayland_display);
    bundle_get_str(kb, "WATCH_WIDTH", &width_str);
    bundle_get_str(kb, "WATCH_HEIGHT", &height_str);

    if (xdg_runtime_dir) {
      LOGGER(DEBUG) << "senenv: " << xdg_runtime_dir;
      setenv("XDG_RUNTIME_DIR", xdg_runtime_dir, 1);
    } else {
      LOGGER(ERROR) << "failed to get xdgruntimedir";
    }

    if (wayland_display) {
      LOGGER(DEBUG) << "setenv: " << wayland_display;
      setenv("WAYLAND_DISPLAY", wayland_display, 1);
    } else {
      LOGGER(ERROR) << "failed to get waylanddisplay";
    }
    bundle_free(kb);
  } else {
    LOGGER(ERROR) << "failed to get launch argv";
  }
  return 0;
}
#endif  // WATCH_FACE_FEATURE_SUPPORT

int real_main(int argc, char* argv[]) {
  STEP_PROFILE_START("Start -> Launch Completed");
  STEP_PROFILE_START("Start -> OnCreate");
  // Parse commandline.
  common::CommandLine::Init(argc, argv);

  common::CommandLine* cmd = common::CommandLine::ForCurrentProcess();
  std::string appid = cmd->GetAppIdFromCommandLine(runtime::kRuntimeExecName);

  // Load Manifest
  auto appdata_manager = common::ApplicationDataManager::GetInstance();
  common::ApplicationData* appdata = appdata_manager->GetApplicationData(appid);
  if (!appdata->LoadManifestData()) {
    return false;
  }

#ifdef WATCH_FACE_FEATURE_SUPPORT
  if (appdata->app_type() == common::ApplicationData::WATCH) {
    setWatchEnv(argc, argv);
  }
#endif  // WATCH_FACE_FEATURE_SUPPORT

  // Default behavior, run as runtime.
  LOGGER(INFO) << "Runtime process has been created.";
  if (!g_prelaunch) {
    ewk_init();
    char* chromium_arg_options[] = {
      argv[0],
      const_cast<char*>("--no-sandbox"),
      const_cast<char*>("--enable-file-cookies"),
      const_cast<char*>("--allow-file-access-from-files"),
      const_cast<char*>("--allow-universal-access-from-files"),
      const_cast<char*>("--single-process")
    };
    const int chromium_arg_cnt =
        sizeof(chromium_arg_options) / sizeof(chromium_arg_options[0]);
    ewk_set_arguments(chromium_arg_cnt, chromium_arg_options);
#ifdef WATCH_FACE_FEATURE_SUPPORT
  } else {
    if (appdata->app_type() == common::ApplicationData::WATCH) {
      // Below code will be enabled after testing
      //ecore_wl_shutdown();
      //ecore_wl_init(NULL);
    }
#endif  // WATCH_FACE_FEATURE_SUPPORT
  }

  int ret = 0;
  // Runtime's destructor should be called before ewk_shutdown()
  {
    std::unique_ptr<runtime::Runtime> runtime =
        runtime::Runtime::MakeRuntime(appdata);
    ret = runtime->Exec(argc, argv);
    runtime.reset();
  }
  ewk_shutdown();
  exit(ret);

  return EXIT_SUCCESS;
}

__attribute__((visibility("default")))
int main(int argc, char* argv[]) {
  if (strcmp(argv[0], "/usr/bin/wrt-loader") == 0) {
    elm_init(argc, argv);
    elm_config_cache_flush_enabled_set(EINA_TRUE);
    auto preload = [argv](void) {
      g_prelaunch = true;
      ewk_init();
      char* chromium_arg_options[] = {
        argv[0],
        const_cast<char*>("--no-sandbox"),
        const_cast<char*>("--enable-file-cookies"),
        const_cast<char*>("--allow-file-access-from-files"),
        const_cast<char*>("--allow-universal-access-from-files"),
        const_cast<char*>("--single-process")
      };
      const int chromium_arg_cnt =
          sizeof(chromium_arg_options) / sizeof(chromium_arg_options[0]);
      ewk_set_arguments(chromium_arg_cnt, chromium_arg_options);
      runtime::PreloadManager::GetInstance()->CreateCacheComponet();
    };
    auto did_launch = [](const std::string& app_path) {
    };
    auto prelaunch = runtime::PreLauncher::Prelaunch;
    return prelaunch(argc, argv, preload, did_launch, real_main);
  } else {
    return real_main(argc, argv);
  }
}
