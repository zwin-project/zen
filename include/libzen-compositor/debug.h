#ifndef LIBZEN_COMPOSIOR_DEBUG_H
#define LIBZEN_COMPOSIOR_DEBUG_H
#ifdef DEBUG

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#include "helpers.h"
#include "timespec-util.h"

static inline void
zen_debug_print_fps(int interval_sec)
{
  static struct timespec base = {0, 0};
  static int count = 0;
  if (base.tv_sec == 0 && base.tv_nsec == 0) {
    timespec_get(&base, TIME_UTC);
  }
  count++;

  struct timespec now;
  timespec_get(&now, TIME_UTC);

  if ((now.tv_sec - base.tv_sec) * 1000000000 + now.tv_nsec - base.tv_nsec >
      1000000000 * interval_sec) {  // 60 hz
    zen_log("%d fps\n", count / interval_sec);
    count = 0;
    base = now;
  }
}

static inline void
zen_debug_start_rec(struct timespec *t)
{
  timespec_get(t, TIME_UTC);
}

static inline void
zen_debug_end_rec(struct timespec *t, const char *label)
{
  struct timespec now;
  int64_t nsec;
  timespec_get(&now, TIME_UTC);
  nsec = timespec_sub_to_nsec(&now, t);
  zen_log("%s takes %ld nano second\n", label, nsec);
}

#ifdef __cplusplus
}
#endif

#endif
#endif  //  LIBZEN_COMPOSIOR_DEBUG_H
