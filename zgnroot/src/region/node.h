#pragma once

#include "cuboid.h"
#include "zgnr/region/node.h"

void zgnr_region_node_add_cuboid(
    struct zgnr_region_node* self, struct zgnr_cuboid_region* cuboid);

struct zgnr_region_node* zgnr_region_node_create_copy(
    struct zgnr_region_node* self);

struct zgnr_region_node* zgnr_region_node_create(void);

void zgnr_region_node_destroy(struct zgnr_region_node* self);
