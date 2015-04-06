# Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Package Dependencies
IF(X11_SUPPORT)
  SET(WIN_PKG "ecore-x")
ENDIF(X11_SUPPORT)
IF(WAYLAND_SUPPORT)
  SET(WIN_PKG "ecore-wayland")
ENDIF(WAYLAND_SUPPORT)

PKG_CHECK_MODULES(TARGET_RUNTIME_DEPS
  dlog
  ${WIN_PKG}
  elementary
  capi-appfw-application
  chromium-efl
  REQUIRED
)

# Include Directories
SET(TARGET_RUNTIME_INCS
  ${BASE_SRCDIR}
  ${TARGET_RUNTIME_DEPS_INCLUDE_DIRS}
)

# Libraries
SET(TARGET_RUNTIME_LIBS
  ${TARGET_RUNTIME_DEPS_LIBRARIES}
)

# Source Files
SET(TARGET_RUNTIME_SRCS
  ${PROJECT_SOURCE_DIR}/src/runtime/command_line.cc
  ${PROJECT_SOURCE_DIR}/src/runtime/native_window.cc
  ${PROJECT_SOURCE_DIR}/src/runtime/native_app_window.cc
  ${PROJECT_SOURCE_DIR}/src/runtime/web_application.cc
  ${PROJECT_SOURCE_DIR}/src/runtime/runtime.cc
  ${PROJECT_SOURCE_DIR}/src/runtime/web_view.cc
)

# Compiler Flags
SET(TARGET_RUNTIME_CFLAGS
  "-fPIE"
)

# Linker Flags
SET(TARGET_RUNTIME_LDFLAGS
  "-pie"
)

# Build Executable
INCLUDE_DIRECTORIES(${TARGET_RUNTIME_INCS})
ADD_DEFINITIONS(${TARGET_RUNTIME_CFLAGS})
ADD_EXECUTABLE(${TARGET_RUNTIME} ${TARGET_RUNTIME_SRCS})
TARGET_LINK_LIBRARIES(${TARGET_RUNTIME}
  "-Wl,--whole-archive"
  ${TARGET_COMMON_STATIC}
  "-Wl,--no-whole-archive"
  ${TARGET_RUNTIME_LIBS}
  ${TARGET_RUNTIME_LDFLAGS}
)

# Install
INSTALL(TARGETS ${TARGET_RUNTIME} DESTINATION bin)
