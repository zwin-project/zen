#pragma once

#include "zen/renderer/dispatcher.h"
#include "zen/renderer/virtual-object.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_rendering_unit;

struct znr_rendering_unit *znr_rendering_unit_create(
    struct znr_dispatcher *dispatcher,
    struct znr_virtual_object *virtual_object);

void znr_rendering_unit_destroy(struct znr_rendering_unit *self);

#ifdef __cplusplus
}
#endif
