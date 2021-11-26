#ifndef ZEN_SHELL_H
#define ZEN_SHELL_H

#include <libzen-compositor/libzen-compositor.h>

extern char* zen_shell_type;

struct zen_shell {
  struct zen_shell_base base;
  struct zen_compositor* compositor;

  struct wl_global* global;
};

#endif  //  ZEN_SHELL_H
