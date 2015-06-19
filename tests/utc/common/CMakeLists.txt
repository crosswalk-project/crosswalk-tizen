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
SET(UTC_COMMON_INCS
  ${BASE_SRCDIR}
  ${GTEST_SRCDIR}/include
  ${UTC_SRCDIR}
  ${UTC_COMMON_DEPS_INCLUDE_DIRS}
)

# Libraries
SET(UTC_COMMON_LIBS
  ${TESTS_GTEST}
  ${TESTS_GTEST_MAIN}
  ${TARGET_COMMON_STATIC}
  ${UTC_COMMON_DEPS_LIBRARIES}
)

# Source Files
SET(UTC_COMMON_SRCS
  ${UTC_SRCDIR}/common/utc_common_url.cc
)

# Compiler Flags
SET(UTC_COMMON_CFLAGS
)

# Linker Flags
SET(UTC_COMMON_LDFLAGS
)

# Build Executable
INCLUDE_DIRECTORIES(${UTC_COMMON_INCS})
ADD_DEFINITIONS(${UTC_COMMON_CFLAGS})
ADD_EXECUTABLE(${UTC_COMMON} ${UTC_COMMON_SRCS})
TARGET_LINK_LIBRARIES(${UTC_COMMON}
  ${UTC_COMMON_LIBS}
  ${UTC_COMMON_LDFLAGS}
)

# Install
install(TARGETS ${UTC_COMMON} DESTINATION bin/${TESTS_UTC})
