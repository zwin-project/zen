#ifndef LIBZEN_VIRTUAL_OBJECT_H
#define LIBZEN_VIRTUAL_OBJECT_H

#include <libzen/libzen.h>
#include <wayland-client.h>

struct zen_virtual_object *zen_virtual_object_create(
    struct wl_client *client, uint32_t id, struct zen_compositor *compositor);

#endif  //  LIBZEN_VIRTUAL_OBJECT_H
