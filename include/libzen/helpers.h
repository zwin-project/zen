#ifndef ZEN_HELPERS_H
#define ZEN_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define UNUSED(x) ((void)x)

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a)[0])
#endif

#ifndef container_of
#define container_of(ptr, type, member)                    \
  ({                                                       \
    const __typeof__(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member));     \
  })
#endif

static inline void *
zalloc(size_t size)
{
  return calloc(1, size);
}

int zen_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif

#endif  //  ZEN_HELPERS_H
