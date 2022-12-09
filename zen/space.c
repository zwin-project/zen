#include "zen/space.h"

#include <zen-common.h>

static void zn_space_destroy(struct zn_space *self);

static void
zn_space_handle_zgnr_space_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_space *self =
      zn_container_of(listener, self, zgnr_space_destroy_listener);

  zn_space_destroy(self);
}

struct zn_space *
zn_space_create(struct zgnr_space *zgnr_space)
{
  struct zn_space *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory ");
    goto err;
  }

  self->zgnr_space = zgnr_space;
  wl_list_init(&self->link);

  wl_signal_init(&self->events.destroy);

  self->zgnr_space_destroy_listener.notify = zn_space_handle_zgnr_space_destroy;
  wl_signal_add(
      &zgnr_space->events.destroy, &self->zgnr_space_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zn_space_destroy(struct zn_space *self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->zgnr_space_destroy_listener.link);
  wl_list_remove(&self->link);
  free(self);
}
