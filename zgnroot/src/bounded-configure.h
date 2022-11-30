#pragma once

#include "zgnr/bounded-configure.h"

struct zgnr_bounded_configure* zgnr_bounded_configure_create(
    struct wl_display* display, vec3 half_size);

void zgnr_bounded_configure_destroy(struct zgnr_bounded_configure* self);
