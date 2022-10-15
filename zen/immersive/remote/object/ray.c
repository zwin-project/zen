#include "zen/immersive/remote/object/ray.h"

#include "zen-common.h"
#include "zen/immersive/remote/scene.h"

static void
zn_ray_remote_object_handle_ray_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_ray_remote_object* self =
      zn_container_of(listener, self, ray_destroy_listener);

  zn_ray_remote_object_destroy(self);
}

struct zn_ray_remote_object*
zn_ray_remote_object_create(
    struct zn_ray* ray, struct zn_remote_scene* remote_scene)
{
  struct zn_ray_remote_object* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->ray = ray;

  self->rendering_unit = znr_rendering_unit_create(remote_scene->remote);
  if (self->rendering_unit == NULL) {
    zn_error("Failed to create a rendering unit");
    goto err_free;
  }

  self->ray_destroy_listener.notify = zn_ray_remote_object_handle_ray_destroy;
  wl_signal_add(&ray->events.destroy, &self->ray_destroy_listener);

  self->remote_scene = remote_scene;
  remote_scene->ray_remote_object = self;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ray_remote_object_destroy(struct zn_ray_remote_object* self)
{
  znr_rendering_unit_destroy(self->rendering_unit);
  self->remote_scene->ray_remote_object = NULL;
  wl_list_remove(&self->ray_destroy_listener.link);
  free(self);
}
