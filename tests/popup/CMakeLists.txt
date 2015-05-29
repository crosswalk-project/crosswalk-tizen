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

SET(TEST_POPUP_SRCDIR ${TEST_SRCDIR}/popup)
SET(BASE_SRCDIR ${PROJECT_SOURCE_DIR}/src)

# Package Dependencies
IF(X11_SUPPORT)
  SET(WIN_PKG "ecore-x")
ENDIF(X11_SUPPORT)
IF(WAYLAND_SUPPORT)
  SET(WIN_PKG "ecore-wayland")
ENDIF(WAYLAND_SUPPORT)

PKG_CHECK_MODULES(TARGET_TEST_POPUP_DEPS
    dlog
    ${WIN_PKG}
    elementary
    ecore
    efl-assist
   REQUIRED)

# Include Directories
SET(TARGET_TEST_POPUP_INCS
  ${BASE_SRCDIR}
  ${TEST_SRCDIR}
  ${TARGET_TEST_POPUP_DEPS_INCLUDE_DIRS}
)

# Libraries
SET(TARGET_TEST_POPUP_LIBS
  ${TARGET_COMMON_STATIC}
  ${TARGET_TEST_POPUP_DEPS_LIBRARIES}
)

# Source Files
SET(TARGET_TEST_POPUP_SRCS
  ${TEST_SRCDIR}/popup/popup.cc
  ${TEST_SRCDIR}/popup/popup_string.cc
  ${TEST_SRCDIR}/popup/test_popup.cc
)

# Compiler Flags
SET(TARGET_TEST_POPUP_CFLAGS
  "-fPIE"
)

# Linker Flags
SET(TARGET_TEST_POPUP_LDFLAGS
  "-pie"
  "-ldl"
)

# Build Executable
SET(TARGET_TEST_POPUP "wrt-popup-test")
INCLUDE_DIRECTORIES(${TARGET_TEST_POPUP_INCS})
ADD_DEFINITIONS(${TARGET_TEST_POPUP_CFLAGS})
ADD_EXECUTABLE(${TARGET_TEST_POPUP} ${TARGET_TEST_POPUP_SRCS})
TARGET_LINK_LIBRARIES(${TARGET_TEST_POPUP}
  ${TARGET_TEST_POPUP_LIBS}
  ${TARGET_TEST_POPUP_LDFLAGS}
)

# Install
INSTALL(TARGETS ${TARGET_TEST_POPUP} DESTINATION bin)
