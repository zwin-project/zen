#pragma once

#include "zen/appearance/system.h"

struct zna_view;
struct zn_view;

enum zna_view_damage_frag {
  ZNA_VIEW_DAMAGE_GEOMETRY = 1 << 0,
  ZNA_VIEW_DAMAGE_TEXTURE = 1 << 1,
};

void zna_view_commit(struct zna_view *self, uint32_t damage);

struct zna_view *zna_view_create(
    struct zn_view *zn_view, struct zna_system *system);

void zna_view_destroy(struct zna_view *self);
