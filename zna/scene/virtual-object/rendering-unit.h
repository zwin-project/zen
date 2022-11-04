#pragma once

#include <zgnr/rendering-unit.h>

struct zna_rendering_unit {
  struct zgnr_rendering_unit* zgnr_rendering_unit;  // nonnull

  struct wl_listener zgnr_rendering_unit_destroy_listener;
};

struct zna_rendering_unit* zna_rendering_unit_create(
    struct zgnr_rendering_unit* zgnr_rendering_unit);
