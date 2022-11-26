#pragma once

#include <wayland-server-core.h>
#include <zgnr/gl-program.h>
#include <zgnr/gl-vertex-array.h>
#include <zgnr/rendering-unit.h>

#ifdef __cplusplus
extern "C" {
#endif

enum zgnr_gl_base_technique_draw_method {
  ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_NONE,
  ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS,
};

union zgnr_gl_base_technique_draw_args {
  struct {
    uint32_t mode;
    int32_t count;
    int32_t first;
  } arrays;
};

struct zgnr_gl_base_technique {
  struct zgnr_rendering_unit *unit;  // nonnull

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  bool comitted;

  struct {
    struct zgnr_gl_vertex_array *vertex_array;  // nullable
    bool vertex_array_changed;

    struct zgnr_gl_program *program;  // nullable
    bool program_changed;

    enum zgnr_gl_base_technique_draw_method draw_method;
    union zgnr_gl_base_technique_draw_args args;
    bool draw_method_changed;

    struct wl_list texture_binding_list;  // zgnr_texture_binding::link
    bool texture_changed;

    // apply from the front
    struct wl_list uniform_variable_list;
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
