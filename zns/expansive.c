#include "expansive.h"

#include <zen-common.h>

static void zns_expansive_destroy(struct zns_expansive *self);

static void
zns_expansive_handle_zgnr_expansive_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zns_expansive *self =
      zn_container_of(listener, self, zgnr_expansive_destroy_listener);

  zns_expansive_destroy(self);
}

struct zns_expansive *
zns_expansive_create(struct zgnr_expansive *zgnr_expansive)
{
  struct zns_expansive *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory ");
    goto err;
  }

  self->zgnr_expansive = zgnr_expansive;

  self->zgnr_expansive_destroy_listener.notify =
      zns_expansive_handle_zgnr_expansive_destroy;
  wl_signal_add(
      &zgnr_expansive->events.destroy, &self->zgnr_expansive_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zns_expansive_destroy(struct zns_expansive *self)
{
  wl_list_remove(&self->zgnr_expansive_destroy_listener.link);
  free(self);
}
