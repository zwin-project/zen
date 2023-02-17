#pragma once

#include <pixman.h>

#include "zen/screen.h"

struct zn_mock_output {
  struct zn_screen *screen;
  pixman_region32_t damage;
};

bool zn_mock_output_damage_contains(struct zn_mock_output *self, int x, int y);

void zn_mock_output_damage_clear(struct zn_mock_output *self);

void zn_mock_output_resize(
    struct zn_mock_output *self, float width, float height);

/// @param width : effective coords
/// @param height : effective coords
struct zn_mock_output *zn_mock_output_create(float width, float height);

void zn_mock_output_destroy(struct zn_mock_output *self);
