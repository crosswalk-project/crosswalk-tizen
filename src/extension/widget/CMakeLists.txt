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

# Generate JS code
ADD_CUSTOM_COMMAND(
  OUTPUT ${BASE_SRCDIR}/extension/widget/widget_api.cc
  COMMAND python
  ARGS ${BASE_SRCDIR}/extension/widget/gen_js.py
  	   ${BASE_SRCDIR}/extension/widget/widget_api.js
       kSource_widget_api
       ${BASE_SRCDIR}/extension/widget/widget_api.cc
  DEPENDS ${BASE_SRCDIR}/extension/widget/widget_api.js
)

PKG_CHECK_MODULES(TARGET_WIDGET_PLUGIN_DEPS REQUIRED
  dlog
)

SET(TARGET_WIDGET_PLUGIN_INCS
  ${BASE_SRCDIR}
  ${TARGET_WIDGET_PLUGIN_DEPS_INCLUDE_DIRS}
)

# Libraries
SET(TARGET_WIDGET_PLUGIN_LIBS
  ${TARGET_WIDGET_PLUGIN_DEPS_LIBRARIES}
)

SET(TARGET_WIDGET_PLUGIN_SRCS
  ${BASE_SRCDIR}/extension/widget/widget.cc
  ${BASE_SRCDIR}/extension/widget/widget_api.cc
)

INCLUDE_DIRECTORIES(${TARGET_WIDGET_PLUGIN_INCS})

ADD_LIBRARY(${TARGET_WIDGET_PLUGIN} SHARED
  ${TARGET_WIDGET_PLUGIN_SRCS}
)

TARGET_LINK_LIBRARIES(${TARGET_WIDGET_PLUGIN}
  ${TARGET_WIDGET_PLUGIN_LIBS}
  ${TARGET_COMMON_STATIC}
)

SET_TARGET_PROPERTIES(${TARGET_WIDGET_PLUGIN} PROPERTIES
    COMPILE_FLAGS -fPIC)
SET_TARGET_PROPERTIES(${TARGET_WIDGET_PLUGIN} PROPERTIES
    LINK_FLAGS "-Wl,--as-needed -Wl,--hash-style=both"
    OUTPUT_NAME ${TARGET_WIDGET_PLUGIN}
)

INSTALL(TARGETS ${TARGET_WIDGET_PLUGIN}
    DESTINATION ${EXTENSION_PATH}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

INSTALL(FILES
    ${BASE_SRCDIR}/extension/widget/widget.json
    DESTINATION ${EXTENSION_PATH}
)
