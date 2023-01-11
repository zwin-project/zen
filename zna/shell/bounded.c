#include "bounded.h"

#include <zen-common.h>

void
zna_bounded_commit(struct zna_bounded *self, uint32_t damage)
{
  zna_bounded_nameplate_unit_commit(
      self->nameplate_unit, self->zns_bounded, self->virtual_object, damage);
}

/**
 * @param session must not be null
 */
static void
zna_bounded_setup_renderer_object(
    struct zna_bounded *self, struct znr_session *session)
{
  UNUSED(session);
  struct znr_dispatcher *dispatcher = self->system->dispatcher;
  self->virtual_object = znr_virtual_object_create(dispatcher);
  zna_bounded_nameplate_unit_setup_renderer_objects(
      self->nameplate_unit, dispatcher, self->virtual_object);

  zna_bounded_commit(self, UINT32_MAX);
}

static void
zna_bounded_teardown_renderer_object(struct zna_bounded *self)
{
  zna_bounded_nameplate_unit_teardown_renderer_objects(self->nameplate_unit);

  if (self->virtual_object) {
    znr_virtual_object_destroy(self->virtual_object);
    self->virtual_object = NULL;
  }
}

static void
zna_bounded_handle_session_created(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_bounded *self =
      zn_container_of(listener, self, session_created_listener);

  struct znr_session *session = self->system->current_session;

  zna_bounded_setup_renderer_object(self, session);
}

static void
zna_bounded_handle_session_destroyed(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_bounded *self =
      zn_container_of(listener, self, session_destroyed_listener);

  zna_bounded_teardown_renderer_object(self);
}

struct zna_bounded *
zna_bounded_create(struct zns_bounded *bounded, struct zna_system *system)
{
  struct zna_bounded *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zns_bounded = bounded;
  self->system = system;
  self->virtual_object = NULL;

  self->nameplate_unit = zna_bounded_nameplate_unit_create(system);
  if (self->nameplate_unit == NULL) {
    zn_error("Failed to create a zna_bounded_nameplate_unit");
    goto err_free;
  }

  self->session_created_listener.notify = zna_bounded_handle_session_created;
  wl_signal_add(
      &system->events.current_session_created, &self->session_created_listener);

  self->session_destroyed_listener.notify =
      zna_bounded_handle_session_destroyed;
  wl_signal_add(&system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  struct znr_session *session = self->system->current_session;
  if (session) {
    zna_bounded_setup_renderer_object(self, session);
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zna_bounded_destroy(struct zna_bounded *self)
{
  if (self->virtual_object) znr_virtual_object_destroy(self->virtual_object);
  zna_bounded_nameplate_unit_destroy(self->nameplate_unit);
  wl_list_remove(&self->session_created_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
