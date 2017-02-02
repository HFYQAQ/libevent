#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "log.h"
#include "style.h"

typedef void (*event_log_cb)(int, const char *);

event_log_cb log_cb = (event_log_cb) 0;

void event_set_log_cb(event_log_cb);
static void _log_helper(int, int, const char *, va_list);
static void _log_tag(int, const char *);

void event_warn(const char *format, ...) {
	va_list list;

	va_start(list, format);
	_log_helper(EVENT_LOG_WARN, errno, format, list);
	va_end(list);
}

void event_warnx(const char *format, ...) {
	va_list list;

	va_start(list, format);
	_log_helper(EVENT_LOG_WARN, -1, format, list);
	va_end(list);
}

void event_err(int eval, const char *format, ...) {
	va_list list;

	va_start(list, format);
	_log_helper(EVENT_LOG_ERR, errno, format, list);
	va_end(list);

	exit(eval);
}

void event_errx(int eval, const char *format, ...) {
	va_list list;

	va_start(list, format);
	_log_helper(EVENT_LOG_ERR, -1, format, list);
	va_end(list);

	exit(eval);
}

void _log_helper(int severity, int log_errno, const char *format, va_list list) {
	char buff[1024];
	size_t len;

	if (format) {
		vsnprintf(buff, sizeof(buff), format, list);
		buff[sizeof(buff) - 1] = '\0';
	} else
		buff[0] = '\0';

	if (log_errno >= 0 && (len < (sizeof(buff) - 1))) {
		len = strlen(buff);
		snprintf(buff + len, sizeof(buff) - len, "%s", strerror(log_errno));
	}
	_log_tag(severity, buff);
}

void event_set_log_cb(event_log_cb cb) {
	log_cb = cb;
}

void _log_tag(int severity, const char *buff) {
	if (log_cb)
		log_cb(severity, buff);
	else {
		char *severity_str;

		switch(severity) {
		case EVENT_LOG_WARN:
			severity_str = FONT_HIGHTLIGHT FONT_COLOR_PURPLE"[warnning]"FONT_NONE;
			break;
		case EVENT_LOG_ERR:
			severity_str = FONT_HIGHTLIGHT FONT_COLOR_RED"[error]"FONT_NONE;
			break;
		default:
			severity_str = FONT_HIGHTLIGHT FONT_COLOR_YELLOW"[unkown]"FONT_NONE;
			break;
		}
		fprintf(stderr, "%s %s\n", severity_str, buff);
	}
}
