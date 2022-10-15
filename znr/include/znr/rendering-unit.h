#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <wayland-server-core.h>

#include "znr/remote.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct znr_rendering_unit {};
#pragma GCC diagnostic pop

struct znr_rendering_unit* znr_rendering_unit_create(struct znr_remote* remote);

void znr_rendering_unit_destroy(struct znr_rendering_unit* self);

#ifdef __cplusplus
}
#endif
