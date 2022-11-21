#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>

#define NSEC_PER_SEC 1000000000

/**
 * r = a - b
 *
 * r, a and b can be the same pointers.
 */
static inline void
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
static inline int64_t
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
static inline int64_t
timespec_to_msec(const struct timespec *a)
{
  return (int64_t)a->tv_sec * 1000 + a->tv_nsec / 1000000;
}

#ifdef __cplusplus
}
#endif
