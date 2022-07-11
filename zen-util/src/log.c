#include <stdarg.h>
#include <stdio.h>

#include "zen-util.h"

ZN_EXPORT int
zn_log(const char *fmt, ...)
{
  int l;
  va_list argp;

  va_start(argp, fmt);
  l = zn_vlog(fmt, argp);
  va_end(argp);

  return l;
}

ZN_EXPORT int
zn_vlog(const char *fmt, va_list ap)
{
  return vfprintf(stderr, fmt, ap);
}
