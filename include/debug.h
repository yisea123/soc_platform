#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <string.h>

typedef enum {
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG
} loglevel_t;


#ifdef DEBUG
#define dbg_printf(format, ...)		printf(format, ##__VA_ARGS__)
#else
#define dbg_printf(format, ...)
#endif


#ifdef LOGGING
#define log_printf(level, format, ...)	printf(format, ##__VA_ARGS__)
#define log_err(format, ...)		log_printf(LOG_ERR, format, ##__VA_ARGS__)
#define log_warning(format, ...)	log_printf(LOG_WARNING, format, ##__VA_ARGS__)
#define log_notice(format, ...)		log_printf(LOG_NOTICE, format, ##__VA_ARGS__)
#define log_info(format, ...)		log_printf(LOG_INFO, format, ##__VA_ARGS__)
#define log_debug(format, ...)		log_printf(LOG_DEBUG, format, ##__VA_ARGS__)
#else
#define log_printf(level, format, ...)
#define log_err(format, ...)
#define log_warning(format, ...)
#define log_notice(format, ...)
#define log_info(format, ...)
#define log_debug(format, ...)
#endif


#ifdef LOGGING
static inline void log_data(void *data, size_t count, size_t limit)
{
	char buffer[128];
	unsigned char *dptr;

	dptr = data;
	if (limit > 0 && count > limit)
		count = limit;
	while (count > 0)
	{
		int len = 0;
		char *ptr = buffer;
		while ((len+3) < sizeof(buffer) && count > 0)
		{
			int n = sprintf(ptr, "%02X ", *dptr++);
			if (n < 0)
				return;
			ptr += n;
			len += n;
			--count;
		}
		log_printf(LOG_INFO, "%s", buffer);
	}
}
#else
static inline void log_data(void *data, size_t count, size_t limit)
{
}
#endif


#endif /* __DEBUG_H__ */
