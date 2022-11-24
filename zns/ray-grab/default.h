#pragma once

#include "zen/ray.h"

struct zns_default_ray_grab {
  struct zn_ray_grab base;
};

void zns_default_ray_grab_init(struct zns_default_ray_grab* self);

void zns_default_ray_grab_fini(struct zns_default_ray_grab* self);
