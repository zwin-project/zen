#pragma once

#include "zen/appearance/system.h"
#include "zen/ray.h"

struct zna_ray;

void zna_ray_commit(struct zna_ray *self);

struct zna_ray *zna_ray_create(struct zn_ray *ray, struct zna_system *system);

void zna_ray_destroy(struct zna_ray *self);
