#pragma once

#include <stdbool.h>
#include <wayland-util.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @return true if successful, false otherwise
bool zn_wl_array_to_int64_t(struct wl_array *array, int64_t *value);

/// @return true if successful, false otherwise
bool zn_wl_array_to_uint64_t(struct wl_array *array, uint64_t *value);

#ifdef __cplusplus
}
#endif
