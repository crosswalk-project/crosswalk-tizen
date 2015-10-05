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

#ifndef XWALK_COMMON_PROFILER_H_
#define XWALK_COMMON_PROFILER_H_

#include <time.h>

#include <map>
#include <memory>
#include <string>

namespace common {

#define PROFILE_START() PrintProfileLog(__FUNCTION__, "START");
#define PROFILE_END() PrintProfileLog(__FUNCTION__, "END");
#define PROFILE(x) PrintProfileLog(__FUNCTION__, x);

void PrintProfileLog(const char* func, const char* tag);

class ScopeProfile {
 public:
  explicit ScopeProfile(const char* step);
  ~ScopeProfile();
  void Reset();
  void End();
 private:
  std::string step_;
  struct timespec start_;
  bool expired_;
};

class StepProfile {
 public:
  static StepProfile* GetInstance();
  void Start(const char* step);
  void End(const char* step);
 private:
  StepProfile();
  ~StepProfile();
  typedef std::map<const std::string,
                   std::unique_ptr<ScopeProfile> > ProfileMapT;
  ProfileMapT map_;
};

}  // namespace common

#define SCOPE_PROFILE() \
  common::ScopeProfile __profile(__FUNCTION__);

#define STEP_PROFILE_START(x) \
  common::StepProfile::GetInstance()->Start(x)

#define STEP_PROFILE_END(x) \
  common::StepProfile::GetInstance()->End(x)


#endif  // XWALK_COMMON_PROFILER_H_
