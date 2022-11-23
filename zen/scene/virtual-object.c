#include "zen/scene/virtual-object.h"

#include <zen-common.h>

#include "zen/appearance/scene/virtual-object.h"
#include "zen/server.h"

static void zn_virtual_object_destroy(struct zn_virtual_object* self);

static void
zn_virtual_object_handle_zgnr_virtual_object_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_virtual_object* self =
      zn_container_of(listener, self, zgnr_virtual_object_destroy_listener);

  zn_virtual_object_destroy(self);
}

struct zn_virtual_object*
zn_virtual_object_create(
    struct zgnr_virtual_object* zgnr_virtual_object, struct zn_scene* scene)
{
  struct zn_virtual_object* self;
  struct zn_server* server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_virtual_object = zgnr_virtual_object;
  zgnr_virtual_object->user_data = self;
  glm_vec3_zero(self->position);
  glm_quat_identity(self->quaternion);

  wl_list_insert(&scene->virtual_object_list, &self->link);

  self->zgnr_virtual_object_destroy_listener.notify =
      zn_virtual_object_handle_zgnr_virtual_object_destroy;
  wl_signal_add(&self->zgnr_virtual_object->events.destroy,
      &self->zgnr_virtual_object_destroy_listener);

  self->appearance = zna_virtual_object_create(self, server->appearance_system);
  if (self->appearance == NULL) {
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_virtual_object_destroy(struct zn_virtual_object* self)
{
  zna_virtual_object_destroy(self->appearance);
  wl_list_remove(&self->link);
  wl_list_remove(&self->zgnr_virtual_object_destroy_listener.link);
  free(self);
}
