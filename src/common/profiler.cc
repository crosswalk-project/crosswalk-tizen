// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/profiler.h"

#include "common/logger.h"
#include "common/string_utils.h"

namespace wrt {

void PrintProfileLog(const char* func, const char* tag) {
  LoggerD("[PROF] [%s] %s:%s", utils::GetCurrentMilliSeconds().c_str(),
                               func, tag);
}

}  // namespace wrt
