#pragma once

#include "zwnr/region/cuboid.h"

struct zwnr_cuboid_region *zwnr_cuboid_region_create(
    vec3 half_size, vec3 center, versor quaternion);

void zwnr_cuboid_region_destroy(struct zwnr_cuboid_region *self);
