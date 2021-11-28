#ifndef ZEN_RENDERER_OPENGL_VERTEX_BUFFER_H
#define ZEN_RENDERER_OPENGL_VERTEX_BUFFER_H

#include <GL/glew.h>
#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>

struct zen_opengl_vertex_buffer {
  GLuint id;
  struct wl_resource* resource;
  struct {
    struct zen_weak_link buffer_link;
  } pending;
};

void zen_opengl_vertex_buffer_commit(
    struct zen_opengl_vertex_buffer* vertex_buffer);

struct zen_opengl_vertex_buffer* zen_opengl_vertex_buffer_create(
    struct wl_client* client, uint32_t id);

#endif  //  ZEN_RENDERER_OPENGL_VERTEX_BUFFER_H
