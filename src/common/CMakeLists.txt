# Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PKG_CHECK_MODULES(TARGET_COMMON_STATIC_DEPS
  uuid
  gio-2.0
  dlog
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
)

INCLUDE_DIRECTORIES(${TARGET_COMMON_STATIC_INCS})
ADD_LIBRARY(${TARGET_COMMON_STATIC} STATIC
  ${TARGET_COMMON_STATIC_SRCS}
)
