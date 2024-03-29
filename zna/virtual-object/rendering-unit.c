#include "rendering-unit.h"

#include <zen-common.h>
#include <zwnr/virtual-object.h>

#include "virtual-object.h"
#include "virtual-object/gl-base-technique.h"
#include "zen/virtual-object.h"

static void zna_rendering_unit_destroy(struct zna_rendering_unit *self);

static struct zna_virtual_object *
zna_rendering_unit_get_virtual_object(struct zna_rendering_unit *self)
{
  struct zn_virtual_object *virtual_object =
      self->zwnr_rendering_unit->virtual_object->user_data;
  return virtual_object->appearance;
}

static void
zna_rendering_unit_handle_session_destroyed(
    struct wl_listener *listener, void *data)
{
  struct zna_rendering_unit *self =
      zn_container_of(listener, self, session_destroyed_listener);
  UNUSED(data);

  if (self->znr_rendering_unit) {
    znr_rendering_unit_destroy(self->znr_rendering_unit);
    self->znr_rendering_unit = NULL;
  }
}

void
zna_rendering_unit_apply_commit(
    struct zna_rendering_unit *self, bool only_damaged)
{
  struct zna_virtual_object *virtual_object =
      zna_rendering_unit_get_virtual_object(self);

  if (self->znr_rendering_unit == NULL) {
    self->znr_rendering_unit = znr_rendering_unit_create(
        self->system->dispatcher, virtual_object->znr_virtual_object);
  }

  if (self->zwnr_rendering_unit->current.technique) {
    struct zna_gl_base_technique *gl_base_technique =
        self->zwnr_rendering_unit->current.technique->user_data;
    zna_gl_base_technique_apply_commit(gl_base_technique, only_damaged);
  }
}

static void
zna_rendering_unit_handle_zwnr_rendering_unit_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_rendering_unit *self =
      zn_container_of(listener, self, zwnr_rendering_unit_destroy_listener);

  zna_rendering_unit_destroy(self);
}

struct zna_rendering_unit *
zna_rendering_unit_create(
    struct zwnr_rendering_unit *zwnr_rendering_unit, struct zna_system *system)
{
  struct zna_rendering_unit *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zwnr_rendering_unit = zwnr_rendering_unit;
  zwnr_rendering_unit->user_data = self;
  self->system = system;
  self->znr_rendering_unit = NULL;

  self->zwnr_rendering_unit_destroy_listener.notify =
      zna_rendering_unit_handle_zwnr_rendering_unit_destroy;
  wl_signal_add(&self->zwnr_rendering_unit->events.destroy,
      &self->zwnr_rendering_unit_destroy_listener);

  self->session_destroyed_listener.notify =
      zna_rendering_unit_handle_session_destroyed;
  wl_signal_add(&self->system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  return self;

err:
  return NULL;
}

static void
zna_rendering_unit_destroy(struct zna_rendering_unit *self)
{
  if (self->znr_rendering_unit)
    znr_rendering_unit_destroy(self->znr_rendering_unit);
  wl_list_remove(&self->session_destroyed_listener.link);
  wl_list_remove(&self->zwnr_rendering_unit_destroy_listener.link);
  free(self);
}
