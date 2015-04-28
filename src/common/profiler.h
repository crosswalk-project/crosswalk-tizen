// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_PROFILER_H_
#define WRT_COMMON_PROFILER_H_

namespace wrt {

#define PROFILE_START() PrintProfileLog(__FUNCTION__, "START");
#define PROFILE_END() PrintProfileLog(__FUNCTION__, "END");
#define PROFILE(x) PrintProfileLog(__FUNCTION__, x);

void PrintProfileLog(const char* func, const char* tag);

}  // namespace wrt

#endif  // WRT_COMMON_PROFILER_H_
