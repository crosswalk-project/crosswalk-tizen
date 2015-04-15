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
  appsvc
  bundle
  dlog
  ${WIN_PKG}
  elementary
  capi-appfw-application
  chromium-efl
  efl-assist
  deviced
  capi-system-runtime-info
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
  ${BASE_SRCDIR}/runtime/app_control.cc
  ${BASE_SRCDIR}/runtime/command_line.cc
  ${BASE_SRCDIR}/runtime/native_window.cc
  ${BASE_SRCDIR}/runtime/native_app_window.cc
  ${BASE_SRCDIR}/runtime/web_application.cc
  ${BASE_SRCDIR}/runtime/runtime.cc
  ${BASE_SRCDIR}/runtime/web_view.cc
  ${BASE_SRCDIR}/runtime/vibration_manager.cc
  ${BASE_SRCDIR}/runtime/locale_manager.cc
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
  ${TARGET_COMMON_STATIC}
  ${TARGET_EXTENSION_STATIC}
  ${TARGET_RUNTIME_LIBS}
  ${TARGET_RUNTIME_LDFLAGS}
)

# Install
INSTALL(TARGETS ${TARGET_RUNTIME} DESTINATION bin)

# EDC Build
ADD_CUSTOM_COMMAND(
    OUTPUT ${BASE_SRCDIR}/runtime/wrt.edj
    COMMAND edje_cc
    ARGS  ${BASE_SRCDIR}/runtime/wrt.edc
          ${BASE_SRCDIR}/runtime/wrt.edj
    DEPENDS ${BASE_SRCDIR}/runtime/wrt.edc
)
ADD_CUSTOM_TARGET(WRT_EDJE ALL
    DEPENDS ${BASE_SRCDIR}/runtime/wrt.edj
)
INSTALL(FILES
    ${BASE_SRCDIR}/runtime/wrt.edj
    DESTINATION share/edje/wrt/
)
