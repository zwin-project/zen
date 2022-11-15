#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_shm_buffer *zgnr_shm_buffer_get(struct wl_resource *resource);

void zgnr_shm_buffer_begin_access(struct zgnr_shm_buffer *buffer);

void zgnr_shm_buffer_end_access(struct zgnr_shm_buffer *buffer);

void *zgnr_shm_buffer_get_data(struct zgnr_shm_buffer *buffer);

ssize_t zgnr_shm_buffer_get_size(struct zgnr_shm_buffer *buffer);

struct zgnr_shm_pool *zgnr_shm_buffer_ref_pool(struct zgnr_shm_buffer *buffer);

void zgnr_shm_pool_unref(struct zgnr_shm_pool *pool);

int zgnr_shm_init(struct wl_display *display);

#ifdef __cplusplus
}
#endif
