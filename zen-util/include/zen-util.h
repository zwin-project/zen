#ifndef ZEN_UTIL_H
#define ZEN_UTIL_H

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

/** logger */
int zn_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif  //  ZEN_UTIL_H
