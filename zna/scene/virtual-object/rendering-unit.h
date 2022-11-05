#pragma once

#include <zgnr/rendering-unit.h>

#include "system.h"
#include "zen/renderer/rendering-unit.h"

struct zna_rendering_unit {
  struct zgnr_rendering_unit* zgnr_rendering_unit;  // nonnull
  struct zna_system* system;                        // nonnull

  // null when current session does not exist
  struct znr_rendering_unit* znr_rendering_unit;

  struct wl_list link;  // zna_virtual_object::unit_list

  struct wl_listener zgnr_rendering_unit_destroy_listener;
};

void zna_rendering_unit_sync(struct zna_rendering_unit* self);

struct zna_rendering_unit* zna_rendering_unit_create(
    struct zgnr_rendering_unit* zgnr_rendering_unit, struct zna_system* system);
