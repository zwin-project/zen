#pragma once

#include <wlr/backend.h>

#include "zen/backend.h"

struct zn_default_backend {
  struct zn_backend base;

  struct wl_display *display;  // @nonnull, @outlive

  struct wlr_backend *wlr_backend;  // @nonnull, @owning
};
