#pragma once

#include "zgnr/backend.h"

struct zgnr_compositor;

struct zgnr_backend_impl {
  struct zgnr_backend base;

  struct wl_display *display;

  struct zgnr_compositor *compositor;  // nonnull, owning
};
