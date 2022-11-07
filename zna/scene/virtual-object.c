#include "virtual-object.h"

#include <zen-common.h>

#include "system.h"
#include "virtual-object/rendering-unit.h"
#include "zen/renderer/session.h"
#include "zen/renderer/system.h"

/**
 * Precondition:
 *  current session exists && the zgnr_virtual_object has been commited
 */
static void
zna_virtual_object_apply_commit(
    struct zna_virtual_object* self, bool only_damaged)
{
  struct znr_session* session = self->system->renderer->current_session;
  struct zgnr_rendering_unit* unit;
  struct zgnr_virtual_object* zgnr_virtual_object =
      self->zn_virtual_object->zgnr_virtual_object;

  if (self->znr_virtual_object == NULL) {
    self->znr_virtual_object = znr_virtual_object_create(session);
  }

  wl_list_for_each (
      unit, &zgnr_virtual_object->current.rendering_unit_list, link) {
    // Rendering units in current.rendering_unit_list has been commited.
    zna_rendering_unit_apply_commit(unit->user_data, only_damaged);
  }

  znr_virtual_object_commit(self->znr_virtual_object);
}

static void
zna_virtual_object_handle_session_created(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zna_virtual_object* self =
      zn_container_of(listener, self, session_created_listener);

  if (self->zn_virtual_object->zgnr_virtual_object->committed)
    zna_virtual_object_apply_commit(self, false);
}

static void
zna_virtual_object_handle_session_destroyed(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zna_virtual_object* self =
      zn_container_of(listener, self, session_destroyed_listener);

  if (self->znr_virtual_object) {
    znr_virtual_object_destroy(self->znr_virtual_object);
    self->znr_virtual_object = NULL;
  }
}

static void
zna_virtual_object_handle_commit(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zna_virtual_object* self =
      zn_container_of(listener, self, commit_listener);
  struct znr_session* session = self->system->renderer->current_session;

  if (session) zna_virtual_object_apply_commit(self, true);
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

  self->session_created_listener.notify =
      zna_virtual_object_handle_session_created;
  wl_signal_add(&self->system->renderer->events.current_session_created,
      &self->session_created_listener);

  self->session_destroyed_listener.notify =
      zna_virtual_object_handle_session_destroyed;
  wl_signal_add(&self->system->renderer->events.current_session_destroyed,
      &self->session_destroyed_listener);

  self->commit_listener.notify = zna_virtual_object_handle_commit;
  wl_signal_add(&self->zn_virtual_object->zgnr_virtual_object->events.committed,
      &self->commit_listener);

  return self;

err:
  return NULL;
}

void
zna_virtual_object_destroy(struct zna_virtual_object* self)
{
  if (self->znr_virtual_object)
    znr_virtual_object_destroy(self->znr_virtual_object);
  wl_list_remove(&self->session_destroyed_listener.link);
  wl_list_remove(&self->session_created_listener.link);
  wl_list_remove(&self->commit_listener.link);
  free(self);
}
