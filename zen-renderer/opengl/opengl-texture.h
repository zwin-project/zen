#ifndef ZEN_RENDERER_OPENGL_TEXTURE_H
#define ZEN_RENDERER_OPENGL_TEXTURE_H

#include <GL/glew.h>
#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>

struct zen_opengl_texture {
  GLuint id;
  struct wl_resource *resource;
  struct {
    struct zen_weak_link buffer_link;
  } pending;

  int32_t width, height;
};

void zen_opengl_texture_commit(struct zen_opengl_texture *texture);

struct zen_opengl_texture *zen_opengl_texture_create(
    struct wl_client *client, uint32_t id);

#endif  //  ZEN_RENDERER_OPENGL_TEXTURE_H
