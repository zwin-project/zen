#pragma once

#include "zwnr/backend.h"

struct zwnr_compositor;

struct zwnr_backend_impl {
  struct zwnr_backend base;

  struct wl_display *display;

  struct zwnr_compositor *compositor;  // nonnull, owning
};
