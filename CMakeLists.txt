#
# Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# Check minimum CMake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Project name
PROJECT(wrt)

INCLUDE(FindPkgConfig)

# Options
OPTION(X11_SUPPORT "Support x11 for window manager" OFF)
OPTION(WAYLAND_SUPPORT "Support wayland for window manager" OFF)
OPTION(PLUGIN_LAZY_LOADING "Support plugins lazy loading" ON)

# Compiler flags
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)
SET(CMAKE_C_FLAGS_PROFILING    "-O2")
SET(CMAKE_CXX_FLAGS_PROFILING  "-O2 -std=c++0x")
SET(CMAKE_C_FLAGS_DEBUG        "-O0 -g")
SET(CMAKE_CXX_FLAGS_DEBUG      "-O0 -std=c++0x -g")
SET(CMAKE_C_FLAGS_RELEASE      "-Os")
SET(CMAKE_CXX_FLAGS_RELEASE    "-Os -std=c++0x -fvisibility-inlines-hidden")
SET(CMAKE_C_FLAGS_CCOV         "-O0 -g --coverage")
SET(CMAKE_CXX_FLAGS_CCOV       "-O0 -std=c++0x -g --coverage")

ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-Wextra")
ADD_DEFINITIONS("-Wno-variadic-macros")

IF(X11_SUPPORT)
  ADD_DEFINITIONS("-DHAVE_X11")
ENDIF(X11_SUPPORT)
IF(WAYLAND_SUPPORT)
  ADD_DEFINITIONS("-DHAVE_WAYLAND")
ENDIF(WAYLAND_SUPPORT)

IF(PLUGIN_LAZY_LOADING)
  ADD_DEFINITIONS("-DPLUGIN_LAZY_LOADING")
ENDIF(PLUGIN_LAZY_LOADING)

IF(NOT EXTENSION_PATH)
  SET(EXTENSION_PATH ${LIB_INSTALL_DIR}/tizen-extensions-crosswalk)
ENDIF(NOT EXTENSION_PATH)
ADD_DEFINITIONS("-DEXTENSION_PATH=\"${EXTENSION_PATH}\"")

IF(NOT INJECTED_BUNDLE_PATH)
  SET(INJECTED_BUNDLE_PATH ${LIB_INSTALL_DIR}/libwrt-injected-bundle.so)
ENDIF(NOT INJECTED_BUNDLE_PATH)
ADD_DEFINITIONS("-DINJECTED_BUNDLE_PATH=\"${INJECTED_BUNDLE_PATH}\"")

# Targets names
SET(TARGET_RUNTIME  "wrt")
SET(TARGET_COMMON_STATIC "wrt-common-static")
SET(TARGET_EXTENSION_STATIC "wrt-extension-static")
SET(TARGET_INJECTED_BUNDLE "wrt-injected-bundle")
SET(TARGET_WIDGET_PLUGIN "widget-plugin")

# Sub Directories
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(tests)
