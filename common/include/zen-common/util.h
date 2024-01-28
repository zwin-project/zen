#pragma once

#include <stddef.h>  // NOLINT(modernize-deprecated-headers)
#include <stdlib.h>  // NOLINT(modernize-deprecated-headers)

#ifdef __cplusplus
extern "C" {
#endif

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

/** Suppress compiler warnings for unused variables, parameters, functions */
#ifndef UNUSED
#if defined(__GNUC__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif
#endif

#ifndef CLANG_ANALYZER_NORETURN
#if __has_feature(attribute_analyzer_noreturn)
#define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
#else
#define CLANG_ANALYZER_NORETURN
#endif
#endif

/** Allocate memory and set to zero */
UNUSED static inline void *
zalloc(size_t size)
{
  return calloc(1, size);  // NOLINT(cppcoreguidelines-*)
}

#define ZN_MAX(a, b) ((a) > (b) ? (a) : (b))
#define ZN_MIN(a, b) ((a) < (b) ? (a) : (b))

/** Retrieve a pointer to a containing struct */
#define zn_container_of(ptr, sample, member) \
  (__typeof__(sample))((char *)(ptr)-offsetof(__typeof__(*(sample)), member))

#ifdef __cplusplus
}
#endif
