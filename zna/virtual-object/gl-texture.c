#include "gl-texture.h"

#include <zen-common.h>

static void zna_gl_texture_destroy(struct zna_gl_texture *self);

static void
zna_gl_texture_handle_session_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_gl_texture *self =
      zn_container_of(listener, self, session_destroyed_listener);

  if (self->znr_gl_texture) {
    znr_gl_texture_destroy(self->znr_gl_texture);
    self->znr_gl_texture = NULL;
  }
}

static void
zna_gl_texture_handle_zgnr_gl_texture_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_gl_texture *self =
      zn_container_of(listener, self, zgnr_gl_texture_destroy_listener);

  zna_gl_texture_destroy(self);
}

void
zna_gl_texture_apply_commit(struct zna_gl_texture *self, bool only_damaged)
{
  if (self->znr_gl_texture == NULL) {
    self->znr_gl_texture =
        znr_gl_texture_create(self->system->dispatcher, self->system->display);
  }

  if (self->zgnr_gl_texture->current.data_damaged || !only_damaged) {
    znr_gl_texture_image_2d(self->znr_gl_texture,
        self->zgnr_gl_texture->current.target,
        self->zgnr_gl_texture->current.level,
        self->zgnr_gl_texture->current.internal_format,
        self->zgnr_gl_texture->current.width,
        self->zgnr_gl_texture->current.height,
        self->zgnr_gl_texture->current.border,
        self->zgnr_gl_texture->current.format,
        self->zgnr_gl_texture->current.type,
        self->zgnr_gl_texture->current.data);
    self->zgnr_gl_texture->current.data_damaged = false;
  }

  if (self->zgnr_gl_texture->current.generate_mipmap_target_damaged ||
      !only_damaged) {
    znr_gl_texture_generate_mipmap(self->znr_gl_texture,
        self->zgnr_gl_texture->current.generate_mipmap_target);
    self->zgnr_gl_texture->current.generate_mipmap_target_damaged = false;
  }
}

struct zna_gl_texture *
zna_gl_texture_create(
    struct zgnr_gl_texture *zgnr_gl_texture, struct zna_system *system)
{
  struct zna_gl_texture *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_gl_texture = zgnr_gl_texture;
  zgnr_gl_texture->user_data = self;
  self->system = system;
  self->znr_gl_texture = NULL;

  self->zgnr_gl_texture_destroy_listener.notify =
      zna_gl_texture_handle_zgnr_gl_texture_destroy;
  wl_signal_add(&self->zgnr_gl_texture->events.destroy,
      &self->zgnr_gl_texture_destroy_listener);

  self->session_destroyed_listener.notify =
      zna_gl_texture_handle_session_destroy;
  wl_signal_add(&self->system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  return self;

err:
  return NULL;
}

static void
zna_gl_texture_destroy(struct zna_gl_texture *self)
{
  if (self->znr_gl_texture) znr_gl_texture_destroy(self->znr_gl_texture);
  wl_list_remove(&self->zgnr_gl_texture_destroy_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
