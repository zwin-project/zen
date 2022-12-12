#pragma once

#include <zgnr/shell.h>

#include "node.h"
#include "ray-grab/default.h"
#include "zen/shell/shell.h"

struct zns_bounded;

struct zn_shell {
  struct zgnr_shell *zgnr_shell;

  struct zns_node *root;

  struct zns_default_ray_grab default_grab;

  struct wl_listener new_bounded_listener;
  struct wl_listener new_expansive_listener;
  struct wl_listener new_board_listener;
};
