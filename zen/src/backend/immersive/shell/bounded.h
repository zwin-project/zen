#pragma once

#include <cglm/types.h>

#include "zen/bounded.h"

struct zn_bounded *zn_bounded_create(struct wl_client *client, uint32_t id,
    struct zn_virtual_object *virtual_object);
