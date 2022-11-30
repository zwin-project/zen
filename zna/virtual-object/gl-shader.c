#include "gl-shader.h"

#include <zen-common.h>
#include <zgnr/shm.h>

static void zna_gl_shader_destroy(struct zna_gl_shader *self);

static void
zna_gl_shader_handle_session_destroyed(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zna_gl_shader *self =
      zn_container_of(listener, self, session_destroyed_listener);

  if (self->znr_gl_shader) {
    znr_gl_shader_destroy(self->znr_gl_shader);
    self->znr_gl_shader = NULL;
  }
}

static void
zna_gl_shader_handle_zgnr_gl_shader_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zna_gl_shader *self =
      zn_container_of(listener, self, zgnr_gl_shader_destroy_listener);

  zna_gl_shader_destroy(self);
}

void
zna_gl_shader_apply_commit(struct zna_gl_shader *self, bool only_damaged)
{
  UNUSED(only_damaged);

  struct znr_session *session = self->system->current_session;

  if (self->znr_gl_shader == NULL) {
    char *source = zgnr_shm_buffer_get_data(self->zgnr_gl_shader->buffer);
    ssize_t length = zgnr_shm_buffer_get_size(self->zgnr_gl_shader->buffer);

    zgnr_shm_buffer_begin_access(self->zgnr_gl_shader->buffer);
    self->znr_gl_shader = znr_gl_shader_create(
        session, source, length, self->zgnr_gl_shader->type);
    zgnr_shm_buffer_end_access(self->zgnr_gl_shader->buffer);
  }
}

struct zna_gl_shader *
zna_gl_shader_create(
    struct zgnr_gl_shader *zgnr_gl_shader, struct zna_system *system)
{
  struct zna_gl_shader *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_gl_shader = zgnr_gl_shader;
  zgnr_gl_shader->user_data = self;
  self->system = system;
  self->znr_gl_shader = NULL;

  self->zgnr_gl_shader_destroy_listener.notify =
      zna_gl_shader_handle_zgnr_gl_shader_destroy;
  wl_signal_add(&self->zgnr_gl_shader->events.destroy,
      &self->zgnr_gl_shader_destroy_listener);

  self->session_destroyed_listener.notify =
      zna_gl_shader_handle_session_destroyed;
  wl_signal_add(&self->system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  return self;

err:
  return NULL;
}

static void
zna_gl_shader_destroy(struct zna_gl_shader *self)
{
  if (self->znr_gl_shader) znr_gl_shader_destroy(self->znr_gl_shader);
  wl_list_remove(&self->zgnr_gl_shader_destroy_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
