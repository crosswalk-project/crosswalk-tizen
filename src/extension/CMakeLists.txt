# Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PKG_CHECK_MODULES(TARGET_EXTENSION_STATIC_DEPS
  dlog
  glib-2.0
  REQUIRED
)

SET(TARGET_EXTENSION_STATIC_INCS
  ${BASE_SRCDIR}
  ${TARGET_EXTENSION_STATIC_DEPS_INCLUDE_DIRS}
)

SET(TARGET_EXTENSION_STATIC_SRCS
  ${BASE_SRCDIR}/extension/extension.cc
  ${BASE_SRCDIR}/extension/extension_instance.cc
  ${BASE_SRCDIR}/extension/extension_adapter.cc
  ${BASE_SRCDIR}/extension/extension_server.cc
)

INCLUDE_DIRECTORIES(${TARGET_EXTENSION_STATIC_INCS})
ADD_LIBRARY(${TARGET_EXTENSION_STATIC} STATIC
  ${TARGET_EXTENSION_STATIC_SRCS}
)
TARGET_LINK_LIBRARIES(${TARGET_EXTENSION_STATIC}
  ${TARGET_EXTENSION_STATIC_DEPS_LIBRARIES}
  ${TARGET_COMMON_STATIC}
)
