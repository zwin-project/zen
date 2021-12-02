#ifndef LIBZEN_COMPOSIOR_VIRTUAL_OBJECT_H
#define LIBZEN_COMPOSIOR_VIRTUAL_OBJECT_H

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-client.h>

void zen_virtual_object_render_commit(
    struct zen_virtual_object *virtual_object);

struct zen_virtual_object *zen_virtual_object_create(
    struct wl_client *client, uint32_t id, struct zen_compositor *compositor);

#endif  //  LIBZEN_COMPOSIOR_VIRTUAL_OBJECT_H
