#pragma once

#include "zen/appearance/system.h"
#include "zen/ray.h"

struct zna_ray;

struct zna_ray* zna_ray_create(struct zn_ray* ray, struct zna_system* system);

void zna_ray_destroy(struct zna_ray* self);
