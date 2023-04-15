#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zn_shm_buffer *zn_shm_buffer_get(struct wl_resource *resource);

void zn_shm_buffer_begin_access(struct zn_shm_buffer *buffer);

void zn_shm_buffer_end_access(struct zn_shm_buffer *buffer);

void *zn_shm_buffer_get_data(struct zn_shm_buffer *buffer);

ssize_t zn_shm_buffer_get_size(struct zn_shm_buffer *buffer);

struct zn_shm_pool *zn_shm_buffer_ref_pool(struct zn_shm_buffer *buffer);

void zn_shm_pool_unref(struct zn_shm_pool *pool);

int zn_shm_init(struct wl_display *display);

#ifdef __cplusplus
}
#endif
