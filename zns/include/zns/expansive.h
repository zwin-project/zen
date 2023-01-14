#pragma once

#include <zwnr/expansive.h>

#include "node.h"

struct zns_expansive {
  struct zwnr_expansive *zwnr_expansive;  // nonnull

  struct zns_node *node;

  struct wl_listener zwnr_expansive_destroy_listener;
};

struct zns_expansive *zns_expansive_create(
    struct zwnr_expansive *zwnr_expansive);
