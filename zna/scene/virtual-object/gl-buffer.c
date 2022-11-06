#include "gl-buffer.h"

#include <zen-common.h>

static void zna_gl_buffer_destroy(struct zna_gl_buffer *self);

static void
zna_gl_buffer_sync(struct zna_gl_buffer *self)
{
  struct znr_session *session = self->system->renderer->current_session;

  if (self->znr_gl_buffer) {
    znr_gl_buffer_destroy(self->znr_gl_buffer);
    self->znr_gl_buffer = NULL;
  }

  if (session) {
    self->znr_gl_buffer = znr_gl_buffer_create(session);
  }
}

static void
zna_gl_buffer_handle_session(struct wl_listener *listener, void *data)
{
  struct zna_gl_buffer *self =
      zn_container_of(listener, self, session_listener);
  UNUSED(data);

  zna_gl_buffer_sync(self);
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

  self->zgnr_gl_buffer_destroy_listener.notify =
      zna_gl_buffer_handle_zgnr_gl_buffer_destroy;
  wl_signal_add(&self->zgnr_gl_buffer->events.destroy,
      &self->zgnr_gl_buffer_destroy_listener);

  self->session_listener.notify = zna_gl_buffer_handle_session;
  wl_signal_add(&self->system->renderer->events.current_session_changed,
      &self->session_listener);

  zna_gl_buffer_sync(self);

  return self;

err:
  return NULL;
}

static void
zna_gl_buffer_destroy(struct zna_gl_buffer *self)
{
  if (self->znr_gl_buffer) znr_gl_buffer_destroy(self->znr_gl_buffer);
  wl_list_remove(&self->zgnr_gl_buffer_destroy_listener.link);
  wl_list_remove(&self->session_listener.link);
  free(self);
}
