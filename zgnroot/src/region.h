#pragma once

#include <wayland-server-core.h>

#include "region/node.h"

struct zgnr_region {
  struct zgnr_region_node *node;
};

struct zgnr_region *zgnr_region_create(struct wl_client *client, uint32_t id);
