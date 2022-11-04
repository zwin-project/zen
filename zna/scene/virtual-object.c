#include "virtual-object.h"

#include <zen-common.h>

#include "system.h"
#include "zen/renderer/session.h"
#include "zen/renderer/system.h"

static void
zna_virtual_object_sync(struct zna_virtual_object* self)
{
  struct znr_session* session = self->system->renderer->current_session;

  if (self->znr_virtual_object) {
    znr_virtual_object_destroy(self->znr_virtual_object);
    self->znr_virtual_object = NULL;
  }

  if (session) self->znr_virtual_object = znr_virtual_object_create(session);
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
    struct zn_virtual_object* virtual_object, struct zna_system* system)
{
  struct zna_virtual_object* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->virtual_object = virtual_object;
  self->system = system;

  self->session_listener.notify = zna_virtual_object_handle_session;
  wl_signal_add(&self->system->renderer->events.current_session_changed,
      &self->session_listener);

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
  free(self);
}
