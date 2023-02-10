#pragma once

#include <wayland-server-core.h>

void zn_signal_emit_mutable(struct wl_signal *signal, void *data);
