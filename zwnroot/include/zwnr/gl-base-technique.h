#pragma once

#include <wayland-server-core.h>
#include <zwnr/gl-program.h>
#include <zwnr/gl-vertex-array.h>
#include <zwnr/rendering-unit.h>

#ifdef __cplusplus
extern "C" {
#endif

enum zwnr_gl_base_technique_draw_method {
  ZWNR_GL_BASE_TECHNIQUE_DRAW_METHOD_NONE,
  ZWNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS,
  ZWNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ELEMENTS,
};

union zwnr_gl_base_technique_draw_args {
  struct {
    uint32_t mode;
    int32_t count;
    int32_t first;
  } arrays;

  struct {
    uint32_t mode;
    uint32_t count;
    uint32_t type;
    uint64_t offset;
  } elements;
};

struct zwnr_gl_base_technique {
  struct zwnr_rendering_unit *unit;  // nonnull

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  bool comitted;

  struct {
    struct zwnr_gl_vertex_array *vertex_array;  // nullable
    bool vertex_array_changed;

    struct zwnr_gl_program *program;  // nullable
    bool program_changed;

    enum zwnr_gl_base_technique_draw_method draw_method;
    union zwnr_gl_base_technique_draw_args args;
    struct zwnr_gl_buffer *element_array_buffer;  // nullable
    bool draw_method_changed;

    struct wl_list texture_binding_list;  // zwnr_texture_binding::link
    bool texture_changed;

    // apply from the front
    struct wl_list uniform_variable_list;
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
