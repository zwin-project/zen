#pragma once

#include "zgnr/gles-v32.h"

struct zgnr_gles_v32_impl {
  struct zgnr_gles_v32 base;

  struct wl_display *display;

  struct wl_global *global;  // nonnull, owning
};
