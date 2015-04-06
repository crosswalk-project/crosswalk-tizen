# Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

SET(TARGET_COMMON_STATIC_INCS
  ${BASE_SRCDIR}
)

SET(TARGET_COMMON_STATIC_SRCS
  ${PROJECT_SOURCE_DIR}/src/common/file_utils.cc
)

INCLUDE_DIRECTORIES(${TARGET_COMMON_STATIC_INCS})
ADD_LIBRARY(${TARGET_COMMON_STATIC} STATIC
  ${TARGET_COMMON_STATIC_SRCS}
)