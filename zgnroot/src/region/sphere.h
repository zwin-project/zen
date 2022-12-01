#pragma once

#include "zgnr/region/sphere.h"

struct zgnr_sphere_region* zgnr_sphere_region_create(vec3 center, float radius);

void zgnr_sphere_region_destroy(struct zgnr_sphere_region* self);
