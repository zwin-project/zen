#pragma once

#include <zwnr/gles-v32.h>

#include "zen/appearance/system.h"
#include "zen/renderer/dispatcher.h"

struct zna_shader_inventory;

struct zna_system {
  struct wl_display *display;
  struct zwnr_gles_v32 *gles;  // nonnull, owning

  struct zna_shader_inventory *shader_inventory;  // nonnull, owning
  struct znr_session *current_session;            // nullable, owning

  /**
   * Following two dispatcher is not null when current_session exists, null
   * otherwise.
   */
  struct znr_dispatcher *dispatcher;
  struct znr_dispatcher *high_priority_dispatcher;

  struct {
    // When the current session switches to a new one, current_session_destroyed
    // is called first, then current_session_created. That is, the following two
    // signals are always alternately emitted.
    struct wl_signal current_session_created;    // (NULL)
    struct wl_signal current_session_destroyed;  // (NULL)

    struct wl_signal current_session_frame;  // (NULL)
  } events;

  struct wl_listener new_rendering_unit_listener;
  struct wl_listener new_gl_buffer_listener;
  struct wl_listener new_gl_base_technique_listener;
  struct wl_listener new_gl_vertex_array_listener;
  struct wl_listener new_gl_sampler_listener;
  struct wl_listener new_gl_shader_listener;
  struct wl_listener new_gl_program_listener;
  struct wl_listener new_gl_texture_listener;
  struct wl_listener current_session_disconnected_listener;
  struct wl_listener current_session_frame_listener;
};
