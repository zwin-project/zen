#include "zen/immersive/remote/object/ray.h"

#include <GLES3/gl32.h>

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

  self->virtual_object = znr_virtual_object_create(remote_scene->remote);
  if (self->virtual_object == NULL) {
    zn_error("Failed to create a remote virtual object");
    goto err_free;
  }

  self->rendering_unit =
      znr_rendering_unit_create(remote_scene->remote, self->virtual_object->id);
  if (self->rendering_unit == NULL) {
    zn_error("Failed to create a remote rendering unit");
    goto err_virtual_object;
  }

  self->gl_buffer = znr_gl_buffer_create(remote_scene->remote);
  if (self->gl_buffer == NULL) {
    zn_error("Failed to create a remote GL buffer");
    goto err_rendering_unit;
  }

  znr_rendering_unit_gl_enable_vertex_attrib_array(self->rendering_unit, 1);
  znr_rendering_unit_gl_vertex_attrib_pointer(
      self->rendering_unit, 1, self->gl_buffer->id, 3, GL_FLOAT, false, 0, 0);

  znr_virtual_object_commit(self->virtual_object);

  self->ray_destroy_listener.notify = zn_ray_remote_object_handle_ray_destroy;
  wl_signal_add(&ray->events.destroy, &self->ray_destroy_listener);

  self->remote_scene = remote_scene;
  remote_scene->ray_remote_object = self;

  return self;

err_rendering_unit:
  znr_rendering_unit_destroy(self->rendering_unit);

err_virtual_object:
  znr_virtual_object_destroy(self->virtual_object);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ray_remote_object_destroy(struct zn_ray_remote_object* self)
{
  znr_gl_buffer_destroy(self->gl_buffer);
  znr_rendering_unit_destroy(self->rendering_unit);
  znr_virtual_object_destroy(self->virtual_object);
  self->remote_scene->ray_remote_object = NULL;
  wl_list_remove(&self->ray_destroy_listener.link);
  free(self);
}
