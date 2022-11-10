#pragma once

#include <zgnr/gles-v32.h>

#include "zen/appearance/system.h"

struct zna_system {
  struct wl_display *display;
  struct zgnr_gles_v32 *gles;   // nonnull, owning
  struct znr_system *renderer;  // nonnull, reference

  struct wl_listener new_rendering_unit_listener;
  struct wl_listener new_gl_buffer_listener;
  struct wl_listener new_gl_base_technique_listener;
};
