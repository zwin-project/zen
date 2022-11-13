#pragma once

#include "zgnr/gl-vertex-array.h"

struct zgnr_gl_vertex_array_impl {
  struct zgnr_gl_vertex_array base;

  struct {
    struct wl_list vertex_attrib_list;  // struct zgnr_gl_vertex_attrib::link
  } pending;
};

void zgnr_gl_vertex_array_commit(struct zgnr_gl_vertex_array_impl *self);

struct zgnr_gl_vertex_array_impl *zgnr_gl_vertex_array_create(
    struct wl_client *client, uint32_t id);
