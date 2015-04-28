# Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Dependencies
PKG_CHECK_MODULES(TARGET_EXTENSION_DEPS
  uuid
  dlog
  glib-2.0
  gio-2.0
  REQUIRED
)

# Includes
SET(TARGET_EXTENSION_INCS
  ${BASE_SRCDIR}
  ${TARGET_EXTENSION_DEPS_INCLUDE_DIRS}
)

# Libraries
SET(TARGET_EXTENSION_LIBS
  ${TARGET_EXTENSION_DEPS_LIBRARIES}
)

# Source Files
SET(TARGET_EXTENSION_SRCS
  ${BASE_SRCDIR}/extension/extension_main.cc
  ${BASE_SRCDIR}/extension/extension.cc
  ${BASE_SRCDIR}/extension/extension_instance.cc
  ${BASE_SRCDIR}/extension/extension_adapter.cc
  ${BASE_SRCDIR}/extension/extension_server.cc
)

# Compiler Flags
SET(TARGET_EXTENSION_CFLAGS
  "-fPIE"
)

# Linker Flags
SET(TARGET_EXTENSION_LDFLAGS
  "-pie -ldl"
)

# Build Executable
INCLUDE_DIRECTORIES(${TARGET_EXTENSION_INCS})
ADD_DEFINITIONS(${TARGET_EXTENSION_CFLAGS})
ADD_EXECUTABLE(${TARGET_EXTENSION} ${TARGET_EXTENSION_SRCS})
TARGET_LINK_LIBRARIES(${TARGET_EXTENSION}
  ${TARGET_COMMON_STATIC}
  ${TARGET_EXTENSION_LIBS}
  ${TARGET_EXTENSION_LDFLAGS}
)

# Install
INSTALL(TARGETS ${TARGET_EXTENSION} DESTINATION bin)
