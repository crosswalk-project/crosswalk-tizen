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

#ifndef WRT_COMMON_LOGGER_H_
#define WRT_COMMON_LOGGER_H_

#include <dlog.h>
#include <sstream>

#undef LOGGER_TAG
#define LOGGER_TAG "WRT"

#define _LOGGER_LOG(prio, fmt, args...) \
  LOG_(LOG_ID_MAIN, prio, LOGGER_TAG, fmt, ##args)

#define _LOGGER_SLOG(prio, fmt, args...) \
  SECURE_LOG_(LOG_ID_MAIN, prio, LOGGER_TAG, fmt, ##args)

#define LoggerD(fmt, args...) _LOGGER_LOG(DLOG_DEBUG, fmt, ##args)
#define LoggerI(fmt, args...) _LOGGER_LOG(DLOG_INFO, fmt, ##args)
#define LoggerW(fmt, args...) _LOGGER_LOG(DLOG_WARN, fmt, ##args)
#define LoggerE(fmt, args...) _LOGGER_LOG(DLOG_ERROR, fmt, ##args)

#define SLoggerD(fmt, args...) _LOGGER_SLOG(DLOG_DEBUG, fmt, ##args)
#define SLoggerI(fmt, args...) _LOGGER_SLOG(DLOG_INFO, fmt, ##args)
#define SLoggerW(fmt, args...) _LOGGER_SLOG(DLOG_WARN, fmt, ##args)
#define SLoggerE(fmt, args...) _LOGGER_SLOG(DLOG_ERROR, fmt, ##args)

namespace wrt {
namespace utils {

class LogMessageVodify {
 public:
  LogMessageVodify() {}
  void operator&(const std::ostream&) const {}
};

class LogMessage {
 public:
  LogMessage(int severity, const char* tag,
             const char* file, const char* func, const int line)
      : severity_(severity), tag_(tag), file_(file), func_(func), line_(line) {}
  LogMessage(int severity, const char* tag)
      : severity_(severity), tag_(tag), file_(NULL), func_(NULL), line_(0) {}
  ~LogMessage() {
    if (file_) {
      __dlog_print(LOG_ID_MAIN, severity_, tag_,
                   "%s: %s(%d) > %s",
                   file_, func_, line_, stream_.str().c_str());
    } else {
      __dlog_print(LOG_ID_MAIN, severity_, tag_, "%s", stream_.str().c_str());
    }
  }
  std::ostream& stream() { return stream_; }
 private:
  const int severity_;
  const char* tag_;
  const char* file_;
  const char* func_;
  const int line_;
  std::ostringstream stream_;
};

}  // namespace utils
}  // namespace wrt

#ifndef __MODULE__
#define __MODULE__                                                            \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define LOGGER(severity)                                                      \
  wrt::utils::LogMessageVodify() &                                            \
    wrt::utils::LogMessage(DLOG_ ## severity, LOGGER_TAG,                     \
                           __MODULE__, __FUNCTION__, __LINE__).stream()

#define LOGGER_RAW(level, tag)                                                \
  wrt::utils::LogMessageVodify() &                                            \
    wrt::utils::LogMessage(level, tag).stream()


#endif  // WRT_COMMON_LOGGER_H_
