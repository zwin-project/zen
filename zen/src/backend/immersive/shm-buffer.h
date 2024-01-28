#pragma once

#include <sys/types.h>
#include <wayland-server-core.h>

struct zn_buffer;

struct zn_shm_buffer {
  struct wl_resource *resource;  // @nullable, @ref
  int refcount;
  ssize_t size;
  ssize_t offset;
  struct zn_shm_pool *pool;     // @nonnull, @owning(shared)
  struct zn_buffer *zn_buffer;  // @nonnull, @owning
};

struct zn_shm_buffer *zn_shm_buffer_get(struct wl_resource *resource);

struct zn_shm_buffer *zn_shm_buffer_create(struct wl_client *client,
    uint32_t id, int64_t offset, int64_t size,
    struct wl_resource *pool_resource);
