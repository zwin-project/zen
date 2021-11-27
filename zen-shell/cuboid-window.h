#ifndef ZEN_SHELL_CUBOID_WINDOW__H
#define ZEN_SHELL_CUBOID_WINDOW__H

#include <cglm/cglm.h>
#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zen-shell/zen-shell.h>

struct zen_cuboid_window *zen_cuboid_window_create(struct wl_client *client,
    uint32_t id, struct zen_shell *shell,
    struct zen_virtual_object *virtual_object);

void zen_cuboid_window_configure(
    struct zen_cuboid_window *cuboid_window, vec3 half_size);

#endif  //  ZEN_SHELL_CUBOID_WINDOW__H
