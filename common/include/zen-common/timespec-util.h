#pragma once

#include <stdint.h>
#include <time.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
  NSEC_PER_SEC = 1000000000,
  MSEC_PER_SEC = 1000,

  NSEC_PER_MSEC = 1000000,
};

/**
 * r = a - b
 *
 * r, a and b can be the same pointers.
 */
UNUSED static inline void
timespec_sub(
    struct timespec *r, const struct timespec *a, const struct timespec *b)
{
  r->tv_sec = a->tv_sec - b->tv_sec;
  r->tv_nsec = a->tv_nsec - b->tv_nsec;
  if (r->tv_nsec < 0) {
    r->tv_sec--;
    r->tv_nsec += NSEC_PER_SEC;
  }
}

/**
 * Convert timespec to nanoseconds
 *
 * \param a timespec
 * \return nanoseconds
 */
UNUSED static inline int64_t
timespec_to_nsec(const struct timespec *a)
{
  return (int64_t)a->tv_sec * NSEC_PER_SEC + a->tv_nsec;
}

/**
 * Convert timespec to milliseconds
 *
 * \param a timespec
 * \return milliseconds
 */
UNUSED static inline int64_t
timespec_to_msec(const struct timespec *a)
{
  return (int64_t)a->tv_sec * MSEC_PER_SEC + a->tv_nsec / NSEC_PER_MSEC;
}

/**
 * Get current timestamp from CLOCK_REALTIME in ms
 *
 * \return milliseconds
 */
UNUSED static inline int64_t
current_realtime_clock_ms(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return timespec_to_msec(&ts);
}

#ifdef __cplusplus
}
#endif
