#pragma once

#include "zen/ray.h"
#include "zen/shell/shell.h"

struct zns_bounded;

struct zns_default_ray_grab {
  struct zn_ray_grab base;

  struct zn_shell *shell;  // nonnull, reference

  enum wlr_button_state button_state;
};

/**
 * @return NULL if the grab is not zns_default_grab
 */
struct zns_default_ray_grab *zns_default_ray_grab_get(struct zn_ray_grab *grab);

void zns_default_ray_grab_init(
    struct zns_default_ray_grab *self, struct zn_shell *shell);

void zns_default_ray_grab_fini(struct zns_default_ray_grab *self);
