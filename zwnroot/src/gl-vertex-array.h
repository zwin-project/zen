#pragma once

#include "zwnr/gl-vertex-array.h"

struct zwnr_gl_vertex_array_impl {
  struct zwnr_gl_vertex_array base;

  struct {
    struct wl_list vertex_attrib_list;  // struct zwnr_gl_vertex_attrib::link
  } pending;
};

void zwnr_gl_vertex_array_commit(struct zwnr_gl_vertex_array_impl *self);

struct zwnr_gl_vertex_array_impl *zwnr_gl_vertex_array_create(
    struct wl_client *client, uint32_t id);
