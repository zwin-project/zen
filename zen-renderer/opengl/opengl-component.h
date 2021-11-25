#ifndef ZEN_RENDERER_OPENGL_COMPONENT_H
#define ZEN_RENDERER_OPENGL_COMPONENT_H

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

struct zen_opengl_component {
  struct zen_virtual_object *virtual_object;
};

struct zen_opengl_component *zen_opengl_component_create(
    struct wl_client *client, uint32_t id,
    struct zen_virtual_object *virtual_object);

void zen_opengl_component_destroy(struct zen_opengl_component *component);

#endif  //  ZEN_RENDERER_OPENGL_COMPONENT_H
