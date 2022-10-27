#include "zen/immersive/remote/mem-buffer.h"

#include "zen-common.h"

static void zn_remote_mem_buffer_destroy(struct zn_remote_mem_buffer* self);

void
zn_remote_mem_buffer_ref(struct zn_remote_mem_buffer* self)
{
  self->ref++;
}

void
zn_remote_mem_buffer_unref(struct zn_remote_mem_buffer* self)
{
  self->ref--;

  if (self->ref == 0 && self->ref_internal == 0) {
    zn_remote_mem_buffer_destroy(self);
  }
}

static void
zn_remote_mem_buffer_handle_ref(struct wl_listener* listener, void* data)
{
  struct znr_buffer_ref_event* event = data;
  struct zn_remote_mem_buffer* self =
      zn_container_of(listener, self, znr_buffer_ref_listener);

  self->ref_internal = event->count;

  if (self->ref == 0 && self->ref_internal == 0) {
    zn_remote_mem_buffer_destroy(self);
  }
}

struct zn_remote_mem_buffer*
zn_remote_mem_buffer_create(size_t size, struct znr_remote* remote)
{
  struct zn_remote_mem_buffer* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->data = zalloc(size);
  if (self->data == NULL) {
    zn_error("Failed to allocate memory");
    goto err_free;
  }

  self->size = size;
  self->ref = 0;

  self->znr_buffer = znr_buffer_create(self->data, remote);
  if (self->znr_buffer == NULL) {
    zn_error("Failed to create znr_buffer");
    goto err_free_data;
  }

  self->znr_buffer_ref_listener.notify = zn_remote_mem_buffer_handle_ref;
  wl_signal_add(&self->znr_buffer->events.ref, &self->znr_buffer_ref_listener);
  self->ref_internal = 0;

  return self;

err_free_data:
  free(self->data);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_remote_mem_buffer_destroy(struct zn_remote_mem_buffer* self)
{
  znr_buffer_destroy(self->znr_buffer);
  wl_list_remove(&self->znr_buffer_ref_listener.link);
  free(self->data);
  free(self);
}
