#ifndef LIBZEN_OPENGL_COMPONENT_H
#define LIBZEN_OPENGL_COMPONENT_H

#include <wayland-server.h>
#include <zigen-server-protocol.h>

#include "virtual-object.h"

struct zen_opengl_component {
  struct zen_virtual_object *virtual_object;
};

struct zen_opengl_component *zen_opengl_component_create(
    struct wl_client *client, uint32_t id,
    struct zen_virtual_object *virtual_object);

void zen_opengl_component_destroy(struct zen_opengl_component *component);

#endif  //  LIBZEN_OPENGL_COMPONENT_H
