#pragma once

#include "base-unit.h"
#include "zns/bounded.h"

struct zna_bounded_nameplate_unit {
  struct zna_base_unit *base_unit;
};

void zna_bounded_nameplate_unit_commit(struct zna_bounded_nameplate_unit *self,
    struct zns_bounded *bounded, struct znr_virtual_object *virtual_object,
    uint32_t damage);

void zna_bounded_nameplate_unit_setup_renderer_objects(
    struct zna_bounded_nameplate_unit *self, struct znr_dispatcher *dispatcher,
    struct znr_virtual_object *virtual_object);

void zna_bounded_nameplate_unit_teardown_renderer_objects(
    struct zna_bounded_nameplate_unit *self);

struct zna_bounded_nameplate_unit *zna_bounded_nameplate_unit_create(
    struct zna_system *system);

void zna_bounded_nameplate_unit_destroy(
    struct zna_bounded_nameplate_unit *self);
