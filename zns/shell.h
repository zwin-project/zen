#pragma once

#include <zgnr/shell.h>

#include "node.h"
#include "ray-grab/default.h"
#include "zen/shell/shell.h"

struct zns_bounded;
struct zns_seat_capsule;

struct zn_shell {
  struct zgnr_shell *zgnr_shell;

  struct zns_node *root;

  struct zns_node *ray_focus;

  struct zns_seat_capsule *seat_capsule;

  struct zns_default_ray_grab default_grab;

  struct wl_listener new_bounded_listener;
  struct wl_listener new_expansive_listener;
  struct wl_listener new_board_listener;
  struct wl_listener display_system_changed_listener;
  struct wl_listener ray_focus_node_destroy_listener;
};

/**
 * @param node is nonnull
 */
void zn_shell_ray_enter(
    struct zn_shell *self, struct zns_node *node, vec3 origin, vec3 direction);

void zn_shell_ray_clear_focus(struct zn_shell *self);
