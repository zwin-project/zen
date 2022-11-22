#include "bounded.h"

#include <zen-common.h>

static void zns_bounded_destroy(struct zns_bounded* self);

static void
zns_bounded_handle_zgnr_bounded_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);

  struct zns_bounded* self =
      zn_container_of(listener, self, zgnr_bounded_destroy_listener);

  zns_bounded_destroy(self);
}

struct zns_bounded*
zns_bounded_create(struct zgnr_bounded* zgnr_bounded)
{
  struct zns_bounded* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_bounded = zgnr_bounded;

  self->zgnr_bounded_destroy_listener.notify =
      zns_bounded_handle_zgnr_bounded_destroy;
  wl_signal_add(
      &zgnr_bounded->events.destroy, &self->zgnr_bounded_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zns_bounded_destroy(struct zns_bounded* self)
{
  wl_list_remove(&self->zgnr_bounded_destroy_listener.link);
  free(self);
}
