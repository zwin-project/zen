#include "rendering-unit.h"

#include <zen-common.h>
#include <zgnr/virtual-object.h>

#include "scene/virtual-object.h"
#include "zen/scene/virtual-object.h"

static void zna_rendering_unit_destroy(struct zna_rendering_unit* self);

static struct zna_virtual_object*
zna_rendering_unit_get_virtual_object(struct zna_rendering_unit* self)
{
  struct zn_virtual_object* virtual_object =
      self->zgnr_rendering_unit->virtual_object->user_data;
  return virtual_object->appearance;
}

void
zna_rendering_unit_sync(struct zna_rendering_unit* self)
{
  struct znr_session* session = self->system->renderer->current_session;
  struct zna_virtual_object* virtual_object =
      zna_rendering_unit_get_virtual_object(self);

  if (self->znr_rendering_unit) {
    znr_rendering_unit_destroy(self->znr_rendering_unit);
    self->znr_rendering_unit = NULL;
  }

  if (session) {
    self->znr_rendering_unit =
        znr_rendering_unit_create(session, virtual_object->znr_virtual_object);
  }
}

static void
zna_rendering_unit_handle_zgnr_rendering_unit_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zna_rendering_unit* self =
      zn_container_of(listener, self, zgnr_rendering_unit_destroy_listener);

  zna_rendering_unit_destroy(self);
}

struct zna_rendering_unit*
zna_rendering_unit_create(
    struct zgnr_rendering_unit* zgnr_rendering_unit, struct zna_system* system)
{
  struct zna_rendering_unit* self;
  struct zna_virtual_object* zna_virtual_object;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_rendering_unit = zgnr_rendering_unit;
  self->system = system;
  self->znr_rendering_unit = NULL;

  zna_virtual_object = zna_rendering_unit_get_virtual_object(self);
  wl_list_insert(&zna_virtual_object->unit_list, &self->link);

  self->zgnr_rendering_unit_destroy_listener.notify =
      zna_rendering_unit_handle_zgnr_rendering_unit_destroy;
  wl_signal_add(&self->zgnr_rendering_unit->events.destroy,
      &self->zgnr_rendering_unit_destroy_listener);

  zna_rendering_unit_sync(self);

  return self;

err:
  return NULL;
}

static void
zna_rendering_unit_destroy(struct zna_rendering_unit* self)
{
  if (self->znr_rendering_unit)
    znr_rendering_unit_destroy(self->znr_rendering_unit);
  wl_list_remove(&self->link);
  wl_list_remove(&self->zgnr_rendering_unit_destroy_listener.link);
  free(self);
}