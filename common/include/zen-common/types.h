#pragma once

#include <cglm/cglm.h>
#include <sys/types.h>
#include <wayland-util.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @return 0 if successful, -1 otherwise
 */
int zn_array_to_off_t(struct wl_array *array, off_t *value);

/**
 * @return 0 if successful, -1 otherwise
 *
 * @param array is first initialzed by this function
 */
int zn_off_t_to_array(off_t value, struct wl_array *array);

/**
 * @return 0 if successful, -1 otherwise
 */
int zn_array_to_vec3(struct wl_array *array, vec3 vec);

/**
 * @return 0 if successful, -1 otherwise
 *
 * @param array is first initialzed by this function
 */
int zn_vec3_to_array(vec3 vec, struct wl_array *array);

#ifdef __cplusplus
}
#endif
