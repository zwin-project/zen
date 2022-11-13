#pragma once

#include <zgnr/gles-v32.h>

#include "zen/appearance/system.h"

struct zna_system {
  struct wl_display *display;
  struct zgnr_gles_v32 *gles;  // nonnull, owning

  struct znr_session *current_session;  // nullable, owning

  struct {
    // When the current session switches to a new one, current_session_destroyed
    // is called first, then current_session_created. That is, the following two
    // signals are always alternately emitted.
    struct wl_signal current_session_created;    // (NULL)
    struct wl_signal current_session_destroyed;  // (NULL)
  } events;

  struct wl_listener new_rendering_unit_listener;
  struct wl_listener new_gl_buffer_listener;
  struct wl_listener new_gl_base_technique_listener;
  struct wl_listener new_gl_vertex_array_listener;
  struct wl_listener current_session_disconnected_listener;
};
