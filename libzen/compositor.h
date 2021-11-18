#ifndef ZEN_COMPOSITOR_H
#define ZEN_COMPOSITOR_H

#include <libzen/libzen.h>

#include "backend.h"

struct zen_compositor {
  struct wl_display *display;

  struct zen_backend *backend;
};

#endif  //  ZEN_COMPOSITOR_H
