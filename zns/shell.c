#include "shell.h"

#include <float.h>
#include <zen-common.h>
#include <zgnr/bounded.h>

#include "bounded.h"

struct zns_bounded*
zn_shell_ray_cast(struct zn_shell* self, struct zn_ray* ray, float* distance)
{
  struct zns_bounded *bounded, *result_bounded = NULL;

  float min_distance = FLT_MAX;

  wl_list_for_each (bounded, &self->bounded_list, link) {
    float d = zns_bounded_ray_cast(bounded, ray);
    if (d < min_distance) {
      min_distance = d;
      result_bounded = bounded;
    }
  }

  if (min_distance < FLT_MAX) {
    *distance = min_distance;
  }

  return result_bounded;
}

static void
zn_shell_handle_new_bounded(struct wl_listener* listener, void* data)
{
  struct zn_shell* self = zn_container_of(listener, self, new_bounded_listener);

  struct zgnr_bounded* zgnr_bounded = data;

  struct zns_bounded* zns_bounded = zns_bounded_create(zgnr_bounded);

  wl_list_insert(&self->bounded_list, &zns_bounded->link);
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

  zns_default_ray_grab_init(&self->default_grab, self);

  wl_list_init(&self->bounded_list);

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
  wl_list_remove(&self->bounded_list);
  wl_list_remove(&self->new_bounded_listener.link);
  zgnr_shell_destroy(self->zgnr_shell);
  free(self);
}
