#pragma once

#include "zgnr/region/cuboid.h"

struct zgnr_cuboid_region* zgnr_cuboid_region_create(
    vec3 half_size, vec3 center, versor quaternion);

void zgnr_cuboid_region_destroy(struct zgnr_cuboid_region* self);
