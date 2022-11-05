#include "virtual-object.h"

#include <zen-common.h>

#include "system.h"
#include "virtual-object/rendering-unit.h"
#include "zen/renderer/session.h"
#include "zen/renderer/system.h"

static void
zna_virtual_object_sync(struct zna_virtual_object* self)
{
  struct znr_session* session = self->system->renderer->current_session;
  struct zna_rendering_unit* unit;

  if (self->znr_virtual_object) {
    znr_virtual_object_destroy(self->znr_virtual_object);
    self->znr_virtual_object = NULL;
  }

  if (session) self->znr_virtual_object = znr_virtual_object_create(session);

  wl_list_for_each (unit, &self->unit_list, link) {
    zna_rendering_unit_sync(unit);
  }
}

static void
zna_virtual_object_handle_session(struct wl_listener* listener, void* data)
{
  struct zna_virtual_object* self =
      zn_container_of(listener, self, session_listener);
  UNUSED(data);

  zna_virtual_object_sync(self);
}

struct zna_virtual_object*
zna_virtual_object_create(
    struct zn_virtual_object* zn_virtual_object, struct zna_system* system)
{
  struct zna_virtual_object* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->zn_virtual_object = zn_virtual_object;
  self->system = system;

  self->session_listener.notify = zna_virtual_object_handle_session;
  wl_signal_add(&self->system->renderer->events.current_session_changed,
      &self->session_listener);
  wl_list_init(&self->unit_list);

  zna_virtual_object_sync(self);

  return self;

err:
  return NULL;
}

void
zna_virtual_object_destroy(struct zna_virtual_object* self)
{
  if (self->znr_virtual_object)
    znr_virtual_object_destroy(self->znr_virtual_object);
  wl_list_remove(&self->session_listener.link);
  wl_list_remove(&self->unit_list);
  free(self);
}
