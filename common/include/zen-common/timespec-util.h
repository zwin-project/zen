#ifndef ZEN_TIMESPEC_UTIL_H
#define ZEN_TIMESPEC_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

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

/* Convert timespec to nanoseconds
 *
 * \param a timespec
 * \return nanoseconds
 */
static inline int64_t
timespec_to_nsec(const struct timespec *a)
{
  return (int64_t)a->tv_sec * NSEC_PER_SEC + a->tv_nsec;
}

#ifdef __cplusplus
}
#endif

#endif  //  ZEN_TIMESPEC_UTIL_H
