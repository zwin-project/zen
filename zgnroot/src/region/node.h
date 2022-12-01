#pragma once

#include "cuboid.h"
#include "sphere.h"
#include "zgnr/region/node.h"

void zgnr_region_node_add_cuboid(
    struct zgnr_region_node* self, struct zgnr_cuboid_region* cuboid);

void zgnr_region_node_add_sphere(
    struct zgnr_region_node* self, struct zgnr_sphere_region* sphere);

struct zgnr_region_node* zgnr_region_node_create_copy(
    struct zgnr_region_node* self);

struct zgnr_region_node* zgnr_region_node_create(void);

void zgnr_region_node_destroy(struct zgnr_region_node* self);
