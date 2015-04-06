// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_LOGGER_H_
#define WRT_COMMON_LOGGER_H_

#include <dlog.h>

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

#endif  // WRT_COMMON_LOGGER_H_
