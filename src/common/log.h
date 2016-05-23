/*
 * Copyright (c) 2014-2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Lukasz Wojciechowski <l.wojciechow@partner.samsung.com>
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
/**
 * @file        src/common/log.h
 * @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
 * @author      Lukasz Wojciechowski <l.wojciechow@partner.samsung.com>
 * @version     1.0
 * @brief       This file defines logging utilities.
 */


#ifndef _SRC_COMMON_LOG_H
#define _SRC_COMMON_LOG_H

#ifndef ASKUSER_NO_LOGS
#include <sstream>
#ifdef BUILD_WITH_SYSTEMD
#include <systemd/sd-journal.h>
#else // BUILD_WITH_SYSTEMD
#include <syslog.h>
#endif // BUILD_WITH_SYSTEMD
#endif // ASKUSER_NO_LOGS

#define UNUSED __attribute__((unused))

extern int __log_level;

#ifndef ASKUSER_NO_LOGS
namespace {
template <typename ...Args>
void UNUSED __ALOG_FUN(int level, const std::stringstream &format, Args&&... args) {
#ifdef BUILD_WITH_SYSTEMD
  sd_journal_print(level, format.str().c_str(), std::forward<Args>(args)...);
#else // BUILD_WITH_SYSTEMD
  syslog(level, format.str().c_str(), std::forward<Args>(args)...);
#endif // BUILD_WITH_SYSTEMD
}

template <>
void UNUSED __ALOG_FUN(int level, const std::stringstream &format) {
#ifdef BUILD_WITH_SYSTEMD
  sd_journal_print(level, "%s", format.str().c_str());
#else // BUILD_WITH_SYSTEMD
  syslog(level, "%s", format.str().c_str());
#endif // BUILD_WITH_SYSTEMD
}
} // namespace anonymous

#define __ALOG(LEVEL, FORMAT, ...) \
  do { \
  if (LEVEL <= __log_level) { \
  std::stringstream __LOG_FORMAT; \
  __LOG_FORMAT << FORMAT; \
  __ALOG_FUN(LEVEL, __LOG_FORMAT, ##__VA_ARGS__); \
  } \
  } while (0)

#else // ASKUSER_NO_LOGS
#define __ALOG(LEVEL, ...)
#endif

#define ALOGM(...)  __ALOG(LOG_EMERG, __VA_ARGS__)   /* system is unusable */
#define ALOGA(...)  __ALOG(LOG_ALERT, __VA_ARGS__)   /* action must be taken immediately */
#define ALOGC(...)  __ALOG(LOG_CRIT, __VA_ARGS__)    /* critical conditions */
#define ALOGE(...)  __ALOG(LOG_ERR, __VA_ARGS__)     /* error conditions */
#define ALOGW(...)  __ALOG(LOG_WARNING, __VA_ARGS__) /* warning conditions */
#define ALOGN(...)  __ALOG(LOG_NOTICE, __VA_ARGS__)  /* normal but significant condition */
#define ALOGI(...)  __ALOG(LOG_INFO, __VA_ARGS__)    /* informational */
#define ALOGD(...)  __ALOG(LOG_DEBUG, __VA_ARGS__)   /* debug-level messages */

namespace AskUser {

void init_log(void);

} /* namespace AskUser */
#endif /* _SRC_COMMON_LOG_H */
