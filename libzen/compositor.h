#ifndef LIBZEN_COMPOSITOR_H
#define LIBZEN_COMPOSITOR_H

#include <libzen/libzen.h>

#include "backend.h"

#define DEFAULT_REPAINT_WINDOW 7

struct zen_compositor {
  struct wl_display *display;

  struct wl_signal frame_signal;

  struct zen_backend *backend;

  struct wl_event_source *repaint_timer;
  uint32_t repaint_window_msec;
};

#endif  //  LIBZEN_COMPOSITOR_H
