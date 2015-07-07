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

PKG_CHECK_MODULES(TARGET_COMMON_STATIC_DEPS
  uuid
  gio-2.0
  dlog
  capi-appfw-package-manager
  capi-system-system-settings
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
  ${BASE_SRCDIR}/common/url.cc
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
