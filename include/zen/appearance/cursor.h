#pragma once

#include "zen/appearance/system.h"
#include "zen/cursor.h"

struct zna_cursor;

enum zna_cursor_damage_flag {
  ZNA_CURSOR_DAMAGE_GEOMETRY = 1 << 0,
  ZNA_CURSOR_DAMAGE_TEXTURE = 1 << 1,
};

void zna_cursor_commit(struct zna_cursor *self, uint32_t damage);

struct zna_cursor *zna_cursor_create(
    struct zn_cursor *zn_cursor, struct zna_system *system);

void zna_cursor_destroy(struct zna_cursor *self);
