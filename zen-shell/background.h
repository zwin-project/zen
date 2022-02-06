#ifndef ZEN_SHELL_BACKGROUND_H
#define ZEN_SHELL_BACKGROUND_H

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zen-shell/zen-shell.h>

struct zen_background *zen_background_create(struct wl_client *client,
    uint32_t id, struct wl_resource *shell_resource,
    struct zen_virtual_object *virtual_object);

void zen_background_destroy(struct zen_background *background);

#endif  // ZEN_SHELL_BACKGROUND_H
