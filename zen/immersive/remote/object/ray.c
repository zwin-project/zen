#include "zen/immersive/remote/object/ray.h"

#include <GLES3/gl32.h>

#include "zen-common.h"
#include "zen/immersive/remote/scene.h"

#define VERTEX_COUNT 2
#define VERTICES_SIZE sizeof(vec3) * VERTEX_COUNT

static void
zn_ray_remote_object_update_vertices(struct zn_ray_remote_object* self)
{
  vec3* vertices = self->vertex_buffer->data;
  glm_vec3_copy(self->ray->origin, vertices[0]);
  glm_vec3_copy((vec3){0, 1, 0}, vertices[1]);
  glm_vec3_rotate(vertices[1], self->ray->angle.polar, (vec3){0, 0, -1});
  glm_vec3_rotate(vertices[1], self->ray->angle.azimuthal, (vec3){0, 1, 0});
  glm_vec3_add(self->ray->origin, vertices[1], vertices[1]);
}

static void
zn_ray_remote_object_handle_ray_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_ray_remote_object* self =
      zn_container_of(listener, self, ray_destroy_listener);

  zn_ray_remote_object_destroy(self);
}

static void
zn_ray_remote_object_handle_ray_motion(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_ray_remote_object* self =
      zn_container_of(listener, self, ray_motion_listener);

  // TODO: check that the vertex buffer has released

  zn_ray_remote_object_update_vertices(self);

  znr_gl_buffer_gl_buffer_data(self->gl_buffer, self->vertex_buffer->znr_buffer,
      VERTICES_SIZE, GL_DYNAMIC_DRAW);

  znr_virtual_object_commit(self->virtual_object);
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
    zn_error("Failed to create a virtual object for ray");
    goto err_free;
  }

  self->rendering_unit =
      znr_rendering_unit_create(remote_scene->remote, self->virtual_object->id);
  if (self->rendering_unit == NULL) {
    zn_error("Failed to create a rendering unit for ray");
    goto err_virtual_object;
  }

  self->gl_buffer = znr_gl_buffer_create(remote_scene->remote);
  if (self->gl_buffer == NULL) {
    zn_error("Failed to create a GL buffer for ray");
    goto err_rendering_unit;
  }

  self->vertex_buffer =
      zn_remote_mem_buffer_create(VERTICES_SIZE, remote_scene->remote);
  if (self->vertex_buffer == NULL) {
    zn_error("Failed to create a vertex buffer for ray");
    goto err_gl_buffer;
  }
  zn_remote_mem_buffer_ref(self->vertex_buffer);

  zn_ray_remote_object_update_vertices(self);

  znr_gl_buffer_gl_buffer_data(self->gl_buffer, self->vertex_buffer->znr_buffer,
      VERTICES_SIZE, GL_DYNAMIC_DRAW);

  znr_rendering_unit_gl_enable_vertex_attrib_array(self->rendering_unit, 1);
  znr_rendering_unit_gl_vertex_attrib_pointer(
      self->rendering_unit, 1, self->gl_buffer->id, 3, GL_FLOAT, false, 0, 0);

  znr_virtual_object_commit(self->virtual_object);

  self->ray_destroy_listener.notify = zn_ray_remote_object_handle_ray_destroy;
  wl_signal_add(&ray->events.destroy, &self->ray_destroy_listener);

  self->ray_motion_listener.notify = zn_ray_remote_object_handle_ray_motion;
  wl_signal_add(&ray->events.motion, &self->ray_motion_listener);

  self->remote_scene = remote_scene;
  remote_scene->ray_remote_object = self;

  return self;

err_gl_buffer:
  znr_gl_buffer_destroy(self->gl_buffer);

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
  zn_remote_mem_buffer_unref(self->vertex_buffer);
  znr_gl_buffer_destroy(self->gl_buffer);
  znr_rendering_unit_destroy(self->rendering_unit);
  znr_virtual_object_destroy(self->virtual_object);
  self->remote_scene->ray_remote_object = NULL;
  wl_list_remove(&self->ray_destroy_listener.link);
  wl_list_remove(&self->ray_motion_listener.link);
  free(self);
}
