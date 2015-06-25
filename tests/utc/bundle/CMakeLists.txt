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

# Include Directories
SET(UTC_BUNDLE_INCS
  ${BASE_SRCDIR}
  ${GTEST_SRCDIR}/include
  ${UTC_SRCDIR}
  ${UTC_BUNDLE_DEPS_INCLUDE_DIRS}
)

# Libraries
SET(UTC_BUNDLE_LIBS
  ${TESTS_GTEST}
  ${TESTS_GTEST_MAIN}
  ${TARGET_BUNDLE_STATIC}
  ${UTC_BUNDLE_DEPS_LIBRARIES}
)

# Source Files
SET(UTC_BUNDLE_SRCS
  ${UTC_SRCDIR}/bundle/utc_bundle_xxx.cc
)

# Compiler Flags
SET(UTC_BUNDLE_CFLAGS
)

# Linker Flags
SET(UTC_BUNDLE_LDFLAGS
)

# Build Executable
INCLUDE_DIRECTORIES(${UTC_BUNDLE_INCS})
ADD_DEFINITIONS(${UTC_BUNDLE_CFLAGS})
ADD_EXECUTABLE(${UTC_BUNDLE} ${UTC_BUNDLE_SRCS})
TARGET_LINK_LIBRARIES(${UTC_BUNDLE}
  ${UTC_BUNDLE_LIBS}
  ${UTC_BUNDLE_LDFLAGS}
)

# Install
install(TARGETS ${UTC_BUNDLE} DESTINATION bin/${TESTS_UTC})
