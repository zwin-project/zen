#include "shm-pool.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <zwin-protocol.h>

#include "shm-buffer.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-common/wl-array.h"

static void zn_shm_pool_destroy(struct zn_shm_pool *self);

static void *
zn_shm_pool_grow_mapping(struct zn_shm_pool *self)
{
  void *data = NULL;

#ifdef MREMAP_MAYMOVE
  data = mremap(self->data, self->size, self->new_size, MREMAP_MAYMOVE);
#else
#error Operating system must implement mremap(MREMAP_MAYMOVE)
// TODO: fallback
#endif

  return data;
}

static void
zn_shm_pool_finish_resize(struct zn_shm_pool *self)
{
  if (self->size == self->new_size) {
    return;
  }

  void *data = zn_shm_pool_grow_mapping(self);
  if (data == MAP_FAILED) {  // NOLINT(performance-no-int-to-ptr)
    if (self->resource) {
      wl_resource_post_error(
          self->resource, ZWN_SHM_ERROR_INVALID_FD, "failed mremap");
    }
    return;
  }

  self->data = data;
  self->size = self->new_size;
}

static void
zn_shm_pool_handle_destroy(struct wl_resource *resource)
{
  struct zn_shm_pool *self = wl_resource_get_user_data(resource);

  self->resource = NULL;

  zn_shm_pool_unref(self, false);
}

static void
zn_shm_pool_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

static void
zn_shm_pool_protocol_create_buffer(struct wl_client *client,
    struct wl_resource *resource, uint32_t id, struct wl_array *offset_array,
    struct wl_array *size_array)
{
  int64_t offset = 0;
  int64_t size = 0;

  if (!zn_wl_array_to_int64_t(offset_array, &offset) ||
      !zn_wl_array_to_int64_t(size_array, &size)) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_INVALID_WL_ARRAY_SIZE,
        "invalid wl_array size");
    return;
  }

  zn_shm_buffer_create(client, id, offset, size, resource);
}

static void
zn_shm_pool_protocol_resize(
    struct wl_client *client UNUSED, struct wl_resource *resource, int32_t size)
{
  struct zn_shm_pool *self = wl_resource_get_user_data(resource);

  if (size < self->size) {
    wl_resource_post_error(
        resource, ZWN_SHM_ERROR_INVALID_SIZE, "shrinking pool invalid");
    return;
  }

  self->new_size = size;

  if (self->external_refcount == 0) {
    zn_shm_pool_finish_resize(self);
  }
}

static const struct zwn_shm_pool_interface implementation = {
    .destroy = zn_shm_pool_protocol_destroy,
    .create_buffer = zn_shm_pool_protocol_create_buffer,
    .resize = zn_shm_pool_protocol_resize,
};

void
zn_shm_pool_ref(struct zn_shm_pool *self)
{
  zn_assert(self->internal_refcount + self->external_refcount > 0,
      "invalid refcount status");

  self->external_refcount++;
}

void
zn_shm_pool_unref(struct zn_shm_pool *self, bool external)
{
  if (external) {
    self->external_refcount--;
    zn_assert(self->external_refcount >= 0, "invalid external refcount status");
    if (self->external_refcount == 0) {
      zn_shm_pool_finish_resize(self);
    }
  } else {
    self->internal_refcount--;
    zn_assert(self->external_refcount >= 0, "invalid internal refcount status");
  }

  if (self->internal_refcount + self->external_refcount > 0) {
    return;
  }

  zn_shm_pool_destroy(self);
}

struct zn_shm_pool *
zn_shm_pool_create(struct wl_client *client, uint32_t id, int fd, int64_t size,
    struct wl_resource *error_resource)
{
  if (size <= 0) {
    wl_resource_post_error(
        error_resource, ZWN_SHM_ERROR_INVALID_SIZE, "invalid size");
  }

  struct zn_shm_pool *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  int seals = fcntl(fd, F_GET_SEALS);
  if (seals == -1) {
    seals = 0;
  }

  struct stat stat_buf;
  if ((seals & F_SEAL_SHRINK) && fstat(fd, &stat_buf) >= 0) {
    self->sigbuf_is_impossible = stat_buf.st_size >= size;
  } else {
    self->sigbuf_is_impossible = false;
  }

  self->internal_refcount = 1;
  self->external_refcount = 0;
  self->size = size;
  self->new_size = size;

  self->data = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  if (self->data == MAP_FAILED) {  // NOLINT(performance-no-int-to-ptr)
    wl_resource_post_error(error_resource, ZWN_SHM_ERROR_INVALID_FD,
        "failed mmap fd %d: %s", fd, strerror(errno));
    goto err_free;
  }

  self->resource = wl_resource_create(client, &zwn_shm_pool_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_mmap;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zn_shm_pool_handle_destroy);

  return self;

err_mmap:
  munmap(self->data, self->size);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_shm_pool_destroy(struct zn_shm_pool *self)
{
  munmap(self->data, self->size);
  free(self);
}
