#ifndef ZEN_RENDERER_OPENGL_ELEMENT_ARRAY_BUFFER_H
#define ZEN_RENDERER_OPENGL_ELEMENT_ARRAY_BUFFER_H

#include <GL/glew.h>
#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-opengl-server-protocol.h>

struct zen_opengl_element_array_buffer {
  GLuint id;
  struct wl_resource *resource;
  enum zgn_opengl_element_array_indices_type type;

  struct {
    struct zen_weak_link buffer_link;
    enum zgn_opengl_element_array_indices_type type;
  } pending;
};

void zen_opengl_element_array_buffer_commit(
    struct zen_opengl_element_array_buffer *element_array_buffer);

struct zen_opengl_element_array_buffer *zen_opengl_element_array_buffer_create(
    struct wl_client *client, uint32_t id);

#endif  //  ZEN_RENDERER_OPENGL_ELEMENT_ARRAY_BUFFER_H
