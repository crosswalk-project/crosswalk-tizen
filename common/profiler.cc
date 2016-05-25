/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "common/profiler.h"

#include <math.h>
#include <ttrace.h>

#include "common/logger.h"
#include "common/string_utils.h"

namespace common {

namespace {

void PrintProfileTime(const char* step, const struct timespec& start) {
  struct timespec end;
  clock_gettime(CLOCK_REALTIME, &end);
  int64_t diff_in_milli = (end.tv_sec - start.tv_sec) * 1000
                       + round((end.tv_nsec - start.tv_nsec) * 0.000001);
  std::ostringstream ss;
  ss << "END (" << diff_in_milli << "ms)";
  PrintProfileLog(step, ss.str().c_str());
}

}  //  namespace

void PrintProfileLog(const char* func, const char* tag) {
  LOGGER_RAW(DLOG_DEBUG, LOGGER_TAG)
      << "[PROF] [" << utils::GetCurrentMilliSeconds() << "] "
      << func << ":" << tag;
}

ScopeProfile::ScopeProfile(const char* step, const bool isStep)
  : step_(step), expired_(false), isStep_(isStep) {
  clock_gettime(CLOCK_REALTIME, &start_);

  if (!isStep) {
    // Remove return type and parameter info from __PRETTY_FUNCTION__
    int se = step_.find_first_of('(');
    int ss = step_.find_last_of(' ', se) + 1;
    if (ss < se) {
      step_ = step_.substr(ss, se - ss);
    }
  }

  PrintProfileLog(step_.c_str(), "START");

  if(isStep_)
    traceAsyncBegin(TTRACE_TAG_WEB, 0, "%s%s", "XWALK:", step_.c_str());
  else
    traceBegin(TTRACE_TAG_WEB,"%s%s", "XWALK:", step_.c_str());
}

ScopeProfile::~ScopeProfile() {
  if (!expired_) {
    PrintProfileTime(step_.c_str(), start_);

    if(isStep_)
      traceAsyncEnd(TTRACE_TAG_WEB, 0, "%s%s", "XWALK:", step_.c_str());
    else
      traceEnd(TTRACE_TAG_WEB);
  }
}

void ScopeProfile::Reset() {
  clock_gettime(CLOCK_REALTIME, &start_);
  PrintProfileLog(step_.c_str(), "START-updated");

  if(isStep_)
    traceAsyncEnd(TTRACE_TAG_WEB, 0, "%s%s", "XWALK:", step_.c_str());
  else
    traceEnd(TTRACE_TAG_WEB);
}

void ScopeProfile::End() {
  expired_ = true;
  PrintProfileTime(step_.c_str(), start_);
}

StepProfile* StepProfile::GetInstance() {
  static StepProfile instance;
  return &instance;
}

StepProfile::StepProfile() {
}

StepProfile::~StepProfile() {
}

void StepProfile::Start(const char* step) {
  map_[step].reset(new ScopeProfile(step, true));
}

void StepProfile::End(const char* step) {
  map_[step].reset();
}

}  // namespace common
