#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zwnr_shm_buffer *zwnr_shm_buffer_get(struct wl_resource *resource);

void zwnr_shm_buffer_begin_access(struct zwnr_shm_buffer *buffer);

void zwnr_shm_buffer_end_access(struct zwnr_shm_buffer *buffer);

void *zwnr_shm_buffer_get_data(struct zwnr_shm_buffer *buffer);

ssize_t zwnr_shm_buffer_get_size(struct zwnr_shm_buffer *buffer);

struct zwnr_shm_pool *zwnr_shm_buffer_ref_pool(struct zwnr_shm_buffer *buffer);

void zwnr_shm_pool_unref(struct zwnr_shm_pool *pool);

int zwnr_shm_init(struct wl_display *display);

#ifdef __cplusplus
}
#endif
