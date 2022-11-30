#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_gles_v32 {
  struct {
    struct wl_signal new_gl_base_technique;  // (struct zgnr_gl_base_technique*)
    struct wl_signal new_gl_buffer;          // (struct zgnr_gl_buffer*)
    struct wl_signal new_gl_program;         // (struct zgnr_gl_program*)
    struct wl_signal new_gl_shader;          // (struct zgnr_gl_shader*)
    struct wl_signal new_gl_texture;         // (struct zgnr_gl_texture*)
    struct wl_signal new_gl_vertex_array;    // (struct zgnr_gl_vertex_array*)
    struct wl_signal new_rendering_unit;     // (struct zgnr_rendering_unit*)
  } events;
};

struct zgnr_gles_v32* zgnr_gles_v32_create(struct wl_display* display);

void zgnr_gles_v32_destroy(struct zgnr_gles_v32* self);

#ifdef __cplusplus
}
#endif
