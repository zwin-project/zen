#include "shell.h"

#include <zen-common.h>
#include <zgnr/bounded.h>

#include "bounded.h"

static void
zn_shell_handle_new_bounded(struct wl_listener* listener, void* data)
{
  UNUSED(listener);
  struct zgnr_bounded* bounded = data;

  (void)zns_bounded_create(bounded);
}

struct zn_ray_grab*
zn_shell_get_default_grab(struct zn_shell* self)
{
  return &self->default_grab.base;
}

struct zn_shell*
zn_shell_create(struct wl_display* display)
{
  struct zn_shell* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_shell = zgnr_shell_create(display);
  if (self->zgnr_shell == NULL) {
    zn_error("Failed to create zgnr_shell");
    goto err_free;
  }

  zns_default_ray_grab_init(&self->default_grab);

  self->new_bounded_listener.notify = zn_shell_handle_new_bounded;
  wl_signal_add(
      &self->zgnr_shell->events.new_bounded, &self->new_bounded_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_shell_destroy(struct zn_shell* self)
{
  zns_default_ray_grab_fini(&self->default_grab);
  wl_list_remove(&self->new_bounded_listener.link);
  zgnr_shell_destroy(self->zgnr_shell);
  free(self);
}
