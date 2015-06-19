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

# Dependencies
PKG_CHECK_MODULES(TARGET_INJECTED_BUNDLE_DEPS REQUIRED
  dlog
  glib-2.0
  gio-2.0
  uuid
  elementary
  chromium-efl
)

# Include Directories
SET(TARGET_INJECTED_BUNDLE_INCS
  ${BASE_SRCDIR}
  ${TARGET_INJECTED_BUNDLE_DEPS_INCLUDE_DIRS}
)

# Libraries
SET(TARGET_INJECTED_BUNDLE_LIBS
  ${TARGET_INJECTED_BUNDLE_DEPS_LIBRARIES}
)

# Source Files
SET(TARGET_INJECTED_BUNDLE_SRCS
  ${BASE_SRCDIR}/bundle/injected_bundle.cc
  ${BASE_SRCDIR}/bundle/extension_renderer_controller.cc
  ${BASE_SRCDIR}/bundle/extension_client.cc
  ${BASE_SRCDIR}/bundle/extension_module.cc
  ${BASE_SRCDIR}/bundle/module_system.cc
  ${BASE_SRCDIR}/bundle/runtime_ipc_client.cc
  ${BASE_SRCDIR}/bundle/xwalk_v8tools_module.cc
  ${BASE_SRCDIR}/bundle/widget_module.cc
  ${BASE_SRCDIR}/bundle/object_tools_module.cc
)

# Compiler Flags
SET(TARGET_INJECTED_BUNDLE_CFLAGS
  "-fPIC"
)

# Linker Flags
SET(TARGET_INJECTED_BUNDLE_LDFLAGS
  "-ldl"
)

INCLUDE_DIRECTORIES(${TARGET_INJECTED_BUNDLE_INCS})
ADD_DEFINITIONS(${TARGET_INJECTED_BUNDLE_CFLAGS})
ADD_LIBRARY(${TARGET_INJECTED_BUNDLE} SHARED
  ${TARGET_INJECTED_BUNDLE_SRCS}
)
TARGET_LINK_LIBRARIES(${TARGET_INJECTED_BUNDLE}
  ${TARGET_INJECTED_BUNDLE_LIBS}
  ${TARGET_COMMON_STATIC}
  ${TARGET_INJECTED_BUNDLE_LDFLAGS}
)

SET_TARGET_PROPERTIES(${TARGET_INJECTED_BUNDLE} PROPERTIES
    COMPILE_FLAGS -fPIC)
SET_TARGET_PROPERTIES(${TARGET_INJECTED_BUNDLE} PROPERTIES
    LINK_FLAGS "-Wl,--as-needed -Wl,--hash-style=both"
    OUTPUT_NAME ${TARGET_INJECTED_BUNDLE}
)

INSTALL(TARGETS ${TARGET_INJECTED_BUNDLE}
    DESTINATION ${LIB_INSTALL_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
