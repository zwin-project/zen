#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zwnr_gles_v32 {
  struct {
    struct wl_signal new_gl_base_technique;  // (struct zwnr_gl_base_technique*)
    struct wl_signal new_gl_buffer;          // (struct zwnr_gl_buffer*)
    struct wl_signal new_gl_program;         // (struct zwnr_gl_program*)
    struct wl_signal new_gl_shader;          // (struct zwnr_gl_shader*)
    struct wl_signal new_gl_sampler;         // (struct zwnr_gl_sampler*)
    struct wl_signal new_gl_texture;         // (struct zwnr_gl_texture*)
    struct wl_signal new_gl_vertex_array;    // (struct zwnr_gl_vertex_array*)
    struct wl_signal new_rendering_unit;     // (struct zwnr_rendering_unit*)
  } events;
};

struct zwnr_gles_v32 *zwnr_gles_v32_create(struct wl_display *display);

void zwnr_gles_v32_destroy(struct zwnr_gles_v32 *self);

#ifdef __cplusplus
}
#endif
