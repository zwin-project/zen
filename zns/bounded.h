#pragma once

#include <zgnr/bounded.h>

#include "node.h"
#include "zen/ray.h"

struct zns_bounded {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct zgnr_bounded *zgnr_bounded;  // nonnull

  struct zns_node *node;

  struct wl_listener zgnr_bounded_destroy_listener;
  struct wl_listener move_listener;

  struct wl_list link;  // zn_shell::bounded_list
};

struct zns_bounded *zns_bounded_create(struct zgnr_bounded *zgnr_bounded);
