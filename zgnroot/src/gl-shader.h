#pragma once

#include "zgnr/gl-shader.h"

struct zgnr_gl_shader_impl {
  struct zgnr_gl_shader base;

  struct wl_resource *resource;
  struct wl_listener buffer_destroy_listener;
};

struct zgnr_gl_shader_impl *zgnr_gl_shader_create(struct wl_client *client,
    uint32_t id, struct wl_resource *buffer, uint32_t type);
