#pragma once

#include <wayland-server-core.h>
#include <zen-common/weak-resource.h>

#include "rendering-unit.h"
#include "zgnr/gl-base-technique.h"

/**
 * When the associated rendering unit is destroyed, this object is destroyed and
 * the wl_resource becomes inert (resource::data == NULL).
 */
struct zgnr_gl_base_technique_impl {
  struct zgnr_gl_base_technique base;

  struct wl_resource *resource;

  struct {
    struct zn_weak_resource vertex_array;  // zgnr_gl_vertex_array_impl
    bool vertex_array_changed;

    struct zn_weak_resource program;  // zgnr_gl_program_impl
    bool program_changed;

    enum zgnr_gl_base_technique_draw_method draw_method;
    union zgnr_gl_base_technique_draw_args args;
    bool draw_method_changed;

    struct wl_list texture_binding_list;  // zgnr_texture_binding::link
    bool texture_changed;

    // insert from the back, commit from the front
    struct wl_list uniform_variable_list;
  } pending;

  struct wl_listener rendering_unit_destroy_listener;
  struct wl_listener rendering_unit_commit_listener;

  struct wl_listener current_vertex_array_destroy_listener;
  struct wl_listener current_program_destroy_listener;
};

struct zgnr_gl_base_technique_impl *zgnr_gl_base_technique_create(
    struct wl_client *client, uint32_t id,
    struct zgnr_rendering_unit_impl *unit);
