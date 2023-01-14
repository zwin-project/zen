#pragma once

#include "zwnr/gles-v32.h"

struct zwnr_gles_v32_impl {
  struct zwnr_gles_v32 base;

  struct wl_display *display;

  struct wl_global *global;  // nonnull, owning
};
