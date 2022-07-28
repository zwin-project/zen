#ifndef ZEN_UTIL_H
#define ZEN_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>

/** Visibility attribute */
#if defined(__GNUC__) && __GNUC__ >= 4
#define ZN_EXPORT __attribute__((visibility("default")))
#else
#define ZN_EXPORT
#endif

/** Compile-time computation of number of items in an array */
#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a)[0])
#endif

/** Suppress compiler warnings for unused variables */
#ifndef UNUSED
#define UNUSED(x) ((void)x)
#endif

/** Allocate memory and set to zero */
static inline void *
zalloc(size_t size)
{
  return calloc(1, size);
}

/** Retrieve a pointer to a containing struct */
#define zn_container_of(ptr, sample, member) \
  (__typeof__(sample))((char *)(ptr)-offsetof(__typeof__(*sample), member))

#ifdef __cplusplus
}
#endif

#endif  //  ZEN_UTIL_H
