#pragma once

#include <wayland-server-core.h>

#include "region/node.h"

struct zwnr_region {
  struct zwnr_region_node *node;
};

struct zwnr_region *zwnr_region_create(struct wl_client *client, uint32_t id);
