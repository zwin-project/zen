#include <libzen-compositor/libzen-compositor.h>
#include <stdio.h>

WL_EXPORT int
zen_log(const char *fmt, ...)
{
  int l;
  va_list argp;

  va_start(argp, fmt);
  l = vfprintf(stderr, fmt, argp);
  va_end(argp);

  return l;
}
