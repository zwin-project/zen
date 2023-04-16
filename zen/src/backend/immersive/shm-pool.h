#pragma once

#include <stdbool.h>
#include <sys/types.h>
#include <wayland-server-core.h>

struct zn_shm_pool {
  struct wl_resource *resource;  // @nullable, @ref
  int internal_refcount;
  int external_refcount;
  char *data;  // @nonnull, mapped
  ssize_t size;
  ssize_t new_size;
  bool sigbuf_is_impossible;
};

void zn_shm_pool_ref(struct zn_shm_pool *self);

void zn_shm_pool_unref(struct zn_shm_pool *self, bool external);

struct zn_shm_pool *zn_shm_pool_create(struct wl_client *client, uint32_t id,
    int fd, int64_t size, struct wl_resource *error_resource);
