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
SET_TARGET_PROPERTIES(${TARGET_EXTENSION_STATIC} PROPERTIES
    COMPILE_FLAGS -fPIC)
TARGET_LINK_LIBRARIES(${TARGET_EXTENSION_STATIC}
  ${TARGET_EXTENSION_STATIC_DEPS_LIBRARIES}
  ${TARGET_COMMON_STATIC}
)

ADD_SUBDIRECTORY(widget)
