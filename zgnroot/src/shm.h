#pragma once

#include <wayland-server-core.h>

struct zgn_shm_buffer *zgn_shm_buffer_get(struct wl_resource *resource);

void zgn_shm_buffer_begin_access(struct zgn_shm_buffer *buffer);

void zgn_shm_buffer_end_access(struct zgn_shm_buffer *buffer);

void *zgn_shm_buffer_get_data(struct zgn_shm_buffer *buffer);

ssize_t zgn_shm_buffer_get_size(struct zgn_shm_buffer *buffer);

struct zgn_shm_pool *zgn_shm_buffer_ref_pool(struct zgn_shm_buffer *buffer);

void zgn_shm_pool_unref(struct zgn_shm_pool *pool);

int zgn_shm_init(struct wl_display *display);
