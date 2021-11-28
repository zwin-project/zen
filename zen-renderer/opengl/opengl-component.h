#ifndef ZEN_RENDERER_OPENGL_COMPONENT_H
#define ZEN_RENDERER_OPENGL_COMPONENT_H

#include <GL/glew.h>
#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zen-renderer/opengl-renderer.h>
#include <zigen-server-protocol.h>

struct zen_opengl_component_state {
  struct zen_weak_link vertex_buffer_link;
};

struct zen_opengl_component {
  struct wl_resource *resource;
  struct zen_virtual_object *virtual_object;
  struct wl_list link;

  GLuint vertex_array_id;

  struct zen_opengl_component_state current;
  struct zen_opengl_component_state pending;

  struct wl_listener virtual_object_commit_listener;
  struct wl_listener virtual_object_destroy_listener;
};

struct zen_opengl_component *zen_opengl_component_create(
    struct wl_client *client, uint32_t id, struct zen_opengl_renderer *renderer,
    struct zen_virtual_object *virtual_object);

void zen_opengl_component_destroy(struct zen_opengl_component *component);

void zen_opengl_component_render(struct zen_opengl_component *component,
    struct zen_opengl_renderer_camera *camera);

#endif  //  ZEN_RENDERER_OPENGL_COMPONENT_H
