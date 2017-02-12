#ifndef _LOG_H_
#define _LOG_H_

#include "style.h"

#define LOGABLE 1

#define EVENT_LOG_WARN 0
#define EVENT_LOG_ERR 1
#define EVENT_LOG_LOG 2

#define EVENT_LOG_HEAD FONT_HIGHTLIGHT"%s:%s:%d: "FONT_NONE // "<文件名>:<函数名>:<行号>: "

#define EV_CHECK_FMT(m, n) __attribute__((format(printf, m, n)))

void event_warn(const char *, ...) EV_CHECK_FMT(1, 2);
void event_warnx(const char *, ...) EV_CHECK_FMT(1, 2);
void event_err(int, const char *, ...) EV_CHECK_FMT(2, 3);
void event_errx(int, const char *, ...) EV_CHECK_FMT(2, 3);
void event_log(const char *, ...) EV_CHECK_FMT(1, 2);

#endif
