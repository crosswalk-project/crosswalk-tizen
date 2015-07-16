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
  efl-extension
  deviced
  ecore
  notification
  REQUIRED
)

FIND_PACKAGE(Boost REQUIRED system filesystem)

# Include Directories
SET(TARGET_RUNTIME_INCS
  ${BASE_SRCDIR}
  ${TARGET_RUNTIME_DEPS_INCLUDE_DIRS}
)

# Libraries
SET(TARGET_RUNTIME_LIBS
  ${TARGET_RUNTIME_DEPS_LIBRARIES}
  ${Boost_LIBRARIES}
)

# Source Files
SET(TARGET_RUNTIME_SRCS
  ${BASE_SRCDIR}/runtime/main.cc
  ${BASE_SRCDIR}/runtime/native_window.cc
  ${BASE_SRCDIR}/runtime/native_app_window.cc
  ${BASE_SRCDIR}/runtime/web_application.cc
  ${BASE_SRCDIR}/runtime/runtime.cc
  ${BASE_SRCDIR}/runtime/web_view.cc
  ${BASE_SRCDIR}/runtime/web_view_impl.cc
  ${BASE_SRCDIR}/runtime/vibration_manager.cc
  ${BASE_SRCDIR}/runtime/notification_manager.cc
  ${BASE_SRCDIR}/runtime/popup.cc
  ${BASE_SRCDIR}/runtime/popup_string.cc
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
  ${TARGET_RUNTIME_LIBS}
  ${TARGET_COMMON_STATIC}
  ${TARGET_EXTENSION_STATIC}
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
