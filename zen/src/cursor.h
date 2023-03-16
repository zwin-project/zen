#pragma once

#include <wlr/types/wlr_surface.h>

#include "zen/cursor.h"

/// @param surface is nullable
/// When surface is NULL and force is true, the cursor will be hidden.
/// When surface is NULL and force is false, the cursor image will be the
/// default one.
void zn_cursor_set_surface(struct zn_cursor *self, struct wlr_surface *surface,
    int32_t hotspot_x, int32_t hotspot_y, bool force);

struct zn_cursor *zn_cursor_create(void);

void zn_cursor_destroy(struct zn_cursor *self);
