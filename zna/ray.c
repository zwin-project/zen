#include "ray.h"

#include <GLES3/gl32.h>
#include <zen-common.h>

#include "system.h"

void
zna_ray_commit(struct zna_ray *self)
{
  zna_ray_origin_unit_commit(
      self->origin_unit, self->zn_ray, self->virtual_object);
}

/**
 * @param session must not be null
 */
static void
zna_ray_setup_renderer_objects(
    struct zna_ray *self, struct znr_session *session)
{
  UNUSED(session);
  struct znr_dispatcher *dispatcher = self->system->high_priority_dispatcher;
  self->virtual_object = znr_virtual_object_create(dispatcher);

  zna_ray_origin_unit_setup_renderer_objects(
      self->origin_unit, dispatcher, self->virtual_object);

  znr_virtual_object_commit(self->virtual_object);
}

static void
zna_ray_teardown_renderer_objects(struct zna_ray *self)
{
  zna_ray_origin_unit_teardown_renderer_objects(self->origin_unit);

  if (self->virtual_object) {
    znr_virtual_object_destroy(self->virtual_object);
    self->virtual_object = NULL;
  }
}

static void
zna_ray_handle_session_created(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zna_ray *self =
      zn_container_of(listener, self, session_created_listener);
  struct znr_session *session = self->system->current_session;

  zna_ray_setup_renderer_objects(self, session);
}

static void
zna_ray_handle_session_destroyed(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zna_ray *self =
      zn_container_of(listener, self, session_destroyed_listener);

  zna_ray_teardown_renderer_objects(self);
}

struct zna_ray *
zna_ray_create(struct zn_ray *ray, struct zna_system *system)
{
  struct zna_ray *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zn_ray = ray;
  self->system = system;
  self->virtual_object = NULL;

  self->origin_unit = zna_ray_origin_unit_create(system);
  if (self->origin_unit == NULL) {
    zn_error("Failed to create a zna_ray_origin_unit");
    goto err_free;
  }

  self->session_created_listener.notify = zna_ray_handle_session_created;
  wl_signal_add(
      &system->events.current_session_created, &self->session_created_listener);

  self->session_destroyed_listener.notify = zna_ray_handle_session_destroyed;
  wl_signal_add(&system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  struct znr_session *session = self->system->current_session;
  if (session) {
    zna_ray_setup_renderer_objects(self, session);
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zna_ray_destroy(struct zna_ray *self)
{
  if (self->virtual_object) znr_virtual_object_destroy(self->virtual_object);
  zna_ray_origin_unit_destroy(self->origin_unit);
  wl_list_remove(&self->session_created_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
