#pragma once

#include "zwnr/gl-shader.h"

struct zwnr_gl_shader_impl {
  struct zwnr_gl_shader base;

  struct wl_resource *resource;
  struct wl_listener buffer_destroy_listener;
};

struct zwnr_gl_shader_impl *zwnr_gl_shader_create(struct wl_client *client,
    uint32_t id, struct wl_resource *buffer, uint32_t type);
