#pragma once

#include "zen/backend.h"

struct zn_wlr_backend {
  struct zn_backend base;

  struct wl_display *display;  // @nonnull, @outlive
};
