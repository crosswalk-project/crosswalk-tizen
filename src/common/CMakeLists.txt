# Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PKG_CHECK_MODULES(TARGET_COMMON_STATIC_DEPS
  uuid
  gio-2.0
  dlog
  capi-appfw-package-manager
  capi-system-runtime-info
  capi-appfw-application
  aul
  appsvc
  manifest-parser
  manifest-handlers
  REQUIRED
)

SET(TARGET_COMMON_STATIC_INCS
  ${BASE_SRCDIR}
  ${TARGET_COMMON_STATIC_DEPS_INCLUDE_DIRS}
)

SET(TARGET_COMMON_STATIC_SRCS
  ${BASE_SRCDIR}/common/constants.cc
  ${BASE_SRCDIR}/common/file_utils.cc
  ${BASE_SRCDIR}/common/string_utils.cc
  ${BASE_SRCDIR}/common/command_line.cc
  ${BASE_SRCDIR}/common/dbus_server.cc
  ${BASE_SRCDIR}/common/dbus_client.cc
  ${BASE_SRCDIR}/common/profiler.cc
  ${BASE_SRCDIR}/common/resource_manager.cc
  ${BASE_SRCDIR}/common/application_data.cc
  ${BASE_SRCDIR}/common/app_control.cc
  ${BASE_SRCDIR}/common/locale_manager.cc
  ${BASE_SRCDIR}/common/app_db.cc
)

INCLUDE_DIRECTORIES(${TARGET_COMMON_STATIC_INCS})
ADD_LIBRARY(${TARGET_COMMON_STATIC} STATIC
  ${TARGET_COMMON_STATIC_SRCS}
)
SET_TARGET_PROPERTIES(${TARGET_COMMON_STATIC} PROPERTIES
    COMPILE_FLAGS -fPIC)
TARGET_LINK_LIBRARIES(${TARGET_COMMON_STATIC}
  ${TARGET_COMMON_STATIC_DEPS_LIBRARIES}
)
