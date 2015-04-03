# Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Check minimum CMake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Project name
PROJECT(wrt)

INCLUDE(FindPkgConfig)

# Options
OPTION(X11_SUPPORT "Support x11 for window manager" OFF)
OPTION(WAYLAND_SUPPORT "Support wayland for window manager" OFF)

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

# Targets names
SET(TARGET_RUNTIME  "wrt")
SET(TARGET_COMMON_STATIC "wrt-common-static")
SET(TARGET_EXTENSION "wrt-extension")
SET(TARGET_INJECTED_BUNDLE "wrt-injected-bundle")

# Sub Directories
ADD_SUBDIRECTORY(src)
