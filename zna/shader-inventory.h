#pragma once

#include "system.h"
#include "zen/renderer/gl-shader.h"

enum zna_shader_name {
  ZNA_SHADER_DEFAULT_VERTEX = 0,
  ZNA_SHADER_BOARD_VERTEX,
  ZNA_SHADER_BOARD_FRAGMENT,
  ZNA_SHADER_COLOR_FRAGMENT,
  ZNA_SHADER_CURSOR_VERTEX,
  ZNA_SHADER_CURSOR_FRAGMENT,
  ZNA_SHADER_RAY_VERTEX,
  ZNA_SHADER_COUNT,
};

struct zna_shader_inventory;

/**
 * @return struct znr_gl_shader* : nullable
 */
struct znr_gl_shader *zna_shader_inventory_get(
    struct zna_shader_inventory *self, enum zna_shader_name name);

struct zna_shader_inventory *zna_shader_inventory_create(
    struct zna_system *system);

void zna_shader_inventory_destroy(struct zna_shader_inventory *self);
