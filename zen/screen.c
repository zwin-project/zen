#include "zen/screen.h"

#include <zen-common.h>

void
zn_screen_damage(struct zn_screen *self, struct wlr_fbox *box)
{
  self->implementation->damage(self->user_data, box);
}

void
zn_screen_damage_whole(struct zn_screen *self)
{
  self->implementation->damage_whole(self->user_data);
}

struct zn_screen *
zn_screen_create(
    const struct zn_screen_interface *implementation, void *user_data)
{
  struct zn_screen *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->user_data = user_data;
  self->implementation = implementation;
  wl_list_init(&self->link);
  self->board = NULL;

  wl_signal_init(&self->events.destroy);

  return self;

err:
  return NULL;
}

void
zn_screen_destroy(struct zn_screen *self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->link);
  free(self);
}
