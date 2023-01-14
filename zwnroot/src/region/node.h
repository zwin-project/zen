#pragma once

#include "cuboid.h"
#include "zwnr/region/node.h"

void zwnr_region_node_add_cuboid(
    struct zwnr_region_node *self, struct zwnr_cuboid_region *cuboid);

struct zwnr_region_node *zwnr_region_node_create_copy(
    struct zwnr_region_node *self);

struct zwnr_region_node *zwnr_region_node_create(void);

void zwnr_region_node_destroy(struct zwnr_region_node *self);
