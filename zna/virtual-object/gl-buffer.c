#include "gl-buffer.h"

#include <zen-common.h>

static void zna_gl_buffer_destroy(struct zna_gl_buffer *self);

static void
zna_gl_buffer_handle_session_destroyed(struct wl_listener *listener, void *data)
{
  struct zna_gl_buffer *self =
      zn_container_of(listener, self, session_destroyed_listener);
  UNUSED(data);

  if (self->znr_gl_buffer) {
    znr_gl_buffer_destroy(self->znr_gl_buffer);
    self->znr_gl_buffer = NULL;
  }
}

static void
zna_gl_buffer_handle_zgnr_gl_buffer_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_gl_buffer *self =
      zn_container_of(listener, self, zgnr_gl_buffer_destroy_listener);

  zna_gl_buffer_destroy(self);
}

void
zna_gl_buffer_apply_commit(struct zna_gl_buffer *self, bool only_damaged)
{
  if (self->znr_gl_buffer == NULL) {
    self->znr_gl_buffer =
        znr_gl_buffer_create(self->system->dispatcher, self->system->display);
  }

  if (self->zgnr_gl_buffer->current.data_damaged || !only_damaged) {
    znr_gl_buffer_data(self->znr_gl_buffer,
        self->zgnr_gl_buffer->current.target,
        self->zgnr_gl_buffer->current.data,
        self->zgnr_gl_buffer->current.usage);
    self->zgnr_gl_buffer->current.data_damaged = false;
  }
}

struct zna_gl_buffer *
zna_gl_buffer_create(
    struct zgnr_gl_buffer *zgnr_gl_buffer, struct zna_system *system)
{
  struct zna_gl_buffer *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_gl_buffer = zgnr_gl_buffer;
  zgnr_gl_buffer->user_data = self;
  self->system = system;
  self->znr_gl_buffer = NULL;

  self->zgnr_gl_buffer_destroy_listener.notify =
      zna_gl_buffer_handle_zgnr_gl_buffer_destroy;
  wl_signal_add(&self->zgnr_gl_buffer->events.destroy,
      &self->zgnr_gl_buffer_destroy_listener);

  self->session_destroyed_listener.notify =
      zna_gl_buffer_handle_session_destroyed;
  wl_signal_add(&self->system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  return self;

err:
  return NULL;
}

static void
zna_gl_buffer_destroy(struct zna_gl_buffer *self)
{
  if (self->znr_gl_buffer) znr_gl_buffer_destroy(self->znr_gl_buffer);
  wl_list_remove(&self->zgnr_gl_buffer_destroy_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
