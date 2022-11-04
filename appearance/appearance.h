#pragma once

#include <zgnr/gles-v32.h>

#include "zen/appearance/appearance.h"

struct zn_appearance {
  struct wl_display *display;
  struct zgnr_gles_v32 *gles;  // nonnull, owning

  struct wl_listener new_rendering_unit_listener;
};
