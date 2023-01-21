#pragma once

#include "zen/appearance/system.h"

struct zna_view_child;
struct zn_view_child;

enum zna_view_child_damage_flag {
  ZNA_VIEW_CHILD_DAMAGE_GEOMETRY = 1 << 0,
  ZNA_VIEW_CHILD_DAMAGE_TEXTURE = 1 << 1,
};

void zna_view_child_commit(struct zna_view_child *self, uint32_t damage);

struct zna_view_child *zna_view_child_create(
    struct zn_view_child *zn_view_child, struct zna_system *system);

void zna_view_child_destroy(struct zna_view_child *self);
