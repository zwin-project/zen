#include "zen/bounded-configure.h"

struct zn_bounded_configure *zn_bounded_configure_create(
    uint32_t serial, vec3 half_size);

void zn_bounded_configure_destroy(struct zn_bounded_configure *self);
