#pragma once

#include "zen/ray.h"
#include "zen/shell/shell.h"

struct zns_bounded;

struct zns_default_ray_grab {
  struct zn_ray_grab base;

  struct zn_shell* shell;  // nonnull, reference

  struct zns_bounded* focus;  // nullable, reference

  struct wl_listener focus_destroy_listener;
};

void zns_default_ray_grab_init(
    struct zns_default_ray_grab* self, struct zn_shell* shell);

void zns_default_ray_grab_fini(struct zns_default_ray_grab* self);
