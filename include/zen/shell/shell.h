#pragma once

#include <wayland-server-core.h>

#include "zen/ray.h"

struct zn_shell;
struct zn_scene;

/** Always returns the same pointer */
struct zn_ray_grab *zn_shell_get_default_grab(struct zn_shell *self);

struct zn_shell *zn_shell_create(
    struct wl_display *display, struct zn_scene *scene);

void zn_shell_destroy(struct zn_shell *self);
