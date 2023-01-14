#pragma once

#include <zwnr/rendering-unit.h>

#include "system.h"
#include "zen/renderer/rendering-unit.h"

struct zna_rendering_unit {
  struct zwnr_rendering_unit *zwnr_rendering_unit;  // nonnull
  struct zna_system *system;                        // nonnull

  /**
   * null before the initial commit or when the current session does not exists,
   * nonnull otherwise
   */
  struct znr_rendering_unit *znr_rendering_unit;

  struct wl_listener zwnr_rendering_unit_destroy_listener;
  struct wl_listener session_destroyed_listener;
};

/**
 * Precondition:
 *  Current session exists && the zwnr_rendering_unit has been committed
 */
void zna_rendering_unit_apply_commit(
    struct zna_rendering_unit *self, bool only_damaged);

struct zna_rendering_unit *zna_rendering_unit_create(
    struct zwnr_rendering_unit *zwnr_rendering_unit, struct zna_system *system);
