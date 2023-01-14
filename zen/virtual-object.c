#include "zen/virtual-object.h"

#include <cglm/quat.h>
#include <cglm/vec3.h>
#include <zen-common.h>

#include "zen/appearance/virtual-object.h"
#include "zen/server.h"

static void zn_virtual_object_destroy(struct zn_virtual_object *self);

static void
zn_virtual_object_handle_zwnr_virtual_object_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_virtual_object *self =
      zn_container_of(listener, self, zwnr_virtual_object_destroy_listener);

  zn_virtual_object_destroy(self);
}

void
zn_virtual_object_move(
    struct zn_virtual_object *self, vec3 position, versor quaternion)
{
  glm_vec3_copy(position, self->position);
  glm_vec4_copy(quaternion, self->quaternion);
  glm_mat4_identity(self->model_matrix);
  glm_translate(self->model_matrix, position);
  glm_quat_rotate(self->model_matrix, quaternion, self->model_matrix);
  glm_mat4_inv(self->model_matrix, self->model_invert);
  wl_signal_emit(&self->events.move, NULL);
}

struct zn_virtual_object *
zn_virtual_object_create(struct zwnr_virtual_object *zwnr_virtual_object)
{
  struct zn_virtual_object *self;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zwnr_virtual_object = zwnr_virtual_object;
  zwnr_virtual_object->user_data = self;

  wl_list_init(&self->link);
  wl_signal_init(&self->events.move);

  self->zwnr_virtual_object_destroy_listener.notify =
      zn_virtual_object_handle_zwnr_virtual_object_destroy;
  wl_signal_add(&self->zwnr_virtual_object->events.destroy,
      &self->zwnr_virtual_object_destroy_listener);

  self->appearance = zna_virtual_object_create(self, server->appearance_system);
  if (self->appearance == NULL) {
    goto err_free;
  }

  zn_virtual_object_move(self, GLM_VEC3_ZERO, GLM_QUAT_IDENTITY);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_virtual_object_destroy(struct zn_virtual_object *self)
{
  zna_virtual_object_destroy(self->appearance);
  wl_list_remove(&self->events.move.listener_list);
  wl_list_remove(&self->link);
  wl_list_remove(&self->zwnr_virtual_object_destroy_listener.link);
  free(self);
}
