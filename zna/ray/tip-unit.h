#pragma once

#include "base-unit.h"
#include "zen/ray.h"

struct zna_ray_tip_unit {
  struct zna_base_unit *base_unit;
};

void zna_ray_tip_unit_commit(struct zna_ray_tip_unit *self, struct zn_ray *ray,
    struct znr_virtual_object *virtual_object);

void zna_ray_tip_unit_setup_renderer_objects(struct zna_ray_tip_unit *self,
    struct znr_dispatcher *dispatcher,
    struct znr_virtual_object *virtual_object);

void zna_ray_tip_unit_teardown_renderer_objects(struct zna_ray_tip_unit *self);

struct zna_ray_tip_unit *zna_ray_tip_unit_create(struct zna_system *system);

void zna_ray_tip_unit_destroy(struct zna_ray_tip_unit *self);
