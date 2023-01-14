#include "gl-shader.h"

#include <zen-common.h>
#include <zwnr/shm.h>

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
zna_gl_shader_handle_zwnr_gl_shader_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zna_gl_shader *self =
      zn_container_of(listener, self, zwnr_gl_shader_destroy_listener);

  zna_gl_shader_destroy(self);
}

void
zna_gl_shader_apply_commit(struct zna_gl_shader *self, bool only_damaged)
{
  UNUSED(only_damaged);

  if (self->znr_gl_shader == NULL) {
    char *source = zwnr_shm_buffer_get_data(self->zwnr_gl_shader->buffer);
    ssize_t length = zwnr_shm_buffer_get_size(self->zwnr_gl_shader->buffer);

    zwnr_shm_buffer_begin_access(self->zwnr_gl_shader->buffer);
    self->znr_gl_shader = znr_gl_shader_create(
        self->system->dispatcher, source, length, self->zwnr_gl_shader->type);
    zwnr_shm_buffer_end_access(self->zwnr_gl_shader->buffer);
  }
}

struct zna_gl_shader *
zna_gl_shader_create(
    struct zwnr_gl_shader *zwnr_gl_shader, struct zna_system *system)
{
  struct zna_gl_shader *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zwnr_gl_shader = zwnr_gl_shader;
  zwnr_gl_shader->user_data = self;
  self->system = system;
  self->znr_gl_shader = NULL;

  self->zwnr_gl_shader_destroy_listener.notify =
      zna_gl_shader_handle_zwnr_gl_shader_destroy;
  wl_signal_add(&self->zwnr_gl_shader->events.destroy,
      &self->zwnr_gl_shader_destroy_listener);

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
  wl_list_remove(&self->zwnr_gl_shader_destroy_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
