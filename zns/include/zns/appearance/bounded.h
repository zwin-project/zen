#pragma once

#include "zen/appearance/system.h"
#include "zns/bounded.h"

struct zna_bounded;

enum zna_bounded_damage_flag {
  ZNA_BOUNDED_DAMAGE_GEOMETRY = 1 << 0,
  ZNA_BOUNDED_DAMAGE_STATE = 1 << 1,
};

void zna_bounded_commit(struct zna_bounded *self, uint32_t damage);

struct zna_bounded *zna_bounded_create(
    struct zns_bounded *bounded, struct zna_system *system);

void zna_bounded_destroy(struct zna_bounded *self);
