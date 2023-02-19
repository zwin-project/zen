#pragma once

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ZEN_SILENT = 0,
  ZEN_ERROR = 1,
  ZEN_WARN = 2,
  ZEN_INFO = 3,
  ZEN_DEBUG = 4,
  ZEN_LOG_IMPORTANCE_LAST,
} zn_log_importance_t;

#ifdef __GNUC__
#define ATTRIB_PRINTF(start, end) __attribute__((format(printf, start, end)))
#else
#define ATTRIB_PRINTF(start, end)
#endif

typedef void (*terminate_callback_t)(int exit_code);

void zn_log_init(zn_log_importance_t verbosity, terminate_callback_t terminate);

void zn_log_(zn_log_importance_t verbosity, const char *format, ...)
    ATTRIB_PRINTF(2, 3);

void zn_vlog_(zn_log_importance_t verbosity, const char *format, va_list args)
    ATTRIB_PRINTF(2, 0);

void zn_abort_(const char *format, ...) ATTRIB_PRINTF(1, 2);

bool zn_assert_(bool condition, const char *format, ...) ATTRIB_PRINTF(2, 3);

#define zn_log(verb, fmt, ...) \
  zn_log_(verb, "[zen] [%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define zn_error(fmt, ...) \
  zn_log_(ZEN_ERROR, "[zen] [%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define zn_warn(fmt, ...) \
  zn_log_(ZEN_WARN, "[zen] [%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define zn_info(fmt, ...) \
  zn_log_(ZEN_INFO, "[zen] [%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define zn_debug(fmt, ...) \
  zn_log_(ZEN_DEBUG, "[zen] [%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define zn_log_errno(verb, fmt, ...) \
  zn_log(verb, "[zen] " fmt ": %s", ##__VA_ARGS__, strerror(errno))

#define zn_abort(fmt, ...) \
  zn_abort_("[zen] [%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define zn_assert(cond, fmt, ...)                                          \
  zn_assert_(cond, "[zen] [%s:%d] %s: " fmt, __FILE__, __LINE__, __func__, \
      ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
