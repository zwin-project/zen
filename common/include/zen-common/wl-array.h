#pragma once

#include <cglm/types.h>
#include <stdbool.h>
#include <wayland-util.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @return true if successful, false otherwise
bool zn_wl_array_to_int64_t(struct wl_array *array, int64_t *value);

/// @return true if successful, false otherwise
bool zn_wl_array_to_uint64_t(struct wl_array *array, uint64_t *value);

/// @return true if successful, false otherwise
/// @param array must be initialized
bool zn_wl_array_from_vec3(struct wl_array *array, vec3 v);

#ifdef __cplusplus
}
#endif
