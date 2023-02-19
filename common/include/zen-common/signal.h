#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

void zn_signal_emit_mutable(struct wl_signal *signal, void *data);

#ifdef __cplusplus
}
#endif
