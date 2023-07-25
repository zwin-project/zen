#pragma once

#include "zen/xr-dispatcher.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_xr_dispatcher *zn_xr_dispatcher_create(
    void *impl_data, const struct zn_xr_dispatcher_interface *implementation);

void zn_xr_dispatcher_destroy(struct zn_xr_dispatcher *self);

#ifdef __cplusplus
}
#endif
