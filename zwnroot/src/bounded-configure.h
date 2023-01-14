#pragma once

#include "zwnr/bounded-configure.h"

struct zwnr_bounded_configure *zwnr_bounded_configure_create(
    struct wl_display *display, vec3 half_size);

void zwnr_bounded_configure_destroy(struct zwnr_bounded_configure *self);
