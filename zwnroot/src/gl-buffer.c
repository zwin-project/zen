#include "gl-buffer.h"

#include <zen-common.h>
#include <zwin-gles-v32-protocol.h>
#include <zwin-protocol.h>

#include "mem-storage.h"
#include "zwnr/shm.h"

static void zwnr_gl_buffer_destroy(struct zwnr_gl_buffer_impl *self);

static void
zwnr_gl_buffer_handle_destroy(struct wl_resource *resource)
{
  struct zwnr_gl_buffer_impl *self = wl_resource_get_user_data(resource);

  zwnr_gl_buffer_destroy(self);
}

static void
zwnr_gl_buffer_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zwnr_gl_buffer_protocol_data(struct wl_client *client,
    struct wl_resource *resource, uint32_t target, struct wl_resource *data,
    uint32_t usage)
{
  UNUSED(client);
  struct zwnr_gl_buffer_impl *self = wl_resource_get_user_data(resource);

  zn_weak_resource_link(&self->pending.data, data);
  self->pending.target = target;
  self->pending.usage = usage;
}

static const struct zwn_gl_buffer_interface implementation = {
    .destroy = zwnr_gl_buffer_protocol_destroy,
    .data = zwnr_gl_buffer_protocol_data,
};

void
zwnr_gl_buffer_commit(struct zwnr_gl_buffer_impl *self)
{
  if (!self->pending.data.resource) return;  // nothing to commit

  struct zwnr_shm_buffer *buffer =
      zwnr_shm_buffer_get(self->pending.data.resource);

  if (self->base.current.data) {
    zwnr_mem_storage_unref(self->base.current.data);
    self->base.current.data = NULL;
  }

  void *data = zwnr_shm_buffer_get_data(buffer);
  ssize_t size = zwnr_shm_buffer_get_size(buffer);

  zwnr_shm_buffer_begin_access(buffer);
  self->base.current.data = zwnr_mem_storage_create(data, size);
  zwnr_shm_buffer_end_access(buffer);

  self->base.current.data_damaged = true;
  self->base.current.target = self->pending.target;
  self->base.current.usage = self->pending.usage;

  zwn_buffer_send_release(self->pending.data.resource);
  zn_weak_resource_unlink(&self->pending.data);
}

struct zwnr_gl_buffer_impl *
zwnr_gl_buffer_create(struct wl_client *client, uint32_t id)
{
  struct zwnr_gl_buffer_impl *self;
  struct wl_resource *resource;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  resource = wl_resource_create(client, &zwn_gl_buffer_interface, 1, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      resource, &implementation, self, zwnr_gl_buffer_handle_destroy);

  wl_signal_init(&self->base.events.destroy);
  zn_weak_resource_init(&self->pending.data);
  self->base.current.data_damaged = false;
  self->base.current.data = NULL;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zwnr_gl_buffer_destroy(struct zwnr_gl_buffer_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  if (self->base.current.data) {
    zwnr_mem_storage_unref(self->base.current.data);
  }
  zn_weak_resource_unlink(&self->pending.data);
  wl_list_remove(&self->base.events.destroy.listener_list);
  free(self);
}
