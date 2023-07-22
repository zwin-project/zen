#pragma once

#include "zen/screen.h"

/// Called by the impl object
/// @param size : effective coords
void zn_screen_notify_resize(struct zn_screen *self, vec2 size);

/// Called by the impl object
void zn_screen_notify_frame(struct zn_screen *self, struct timespec *when);

/// Called by the impl object
struct zn_screen *zn_screen_create(
    void *impl_data, const struct zn_screen_interface *implementation);

/// Called by the impl object
void zn_screen_destroy(struct zn_screen *self);
