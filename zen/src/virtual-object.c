#include "zen/virtual-object.h"

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/bounded.h"

bool
zn_virtual_object_set_role(struct zn_virtual_object *self,
    enum zn_virtual_object_role role, void *role_object)
{
  zn_assert(role != ZN_VIRTUAL_OBJECT_ROLE_NONE,
      "Do not pass NONE role to zn_virtual_object_set_role");

  bool valid = self->role == ZN_VIRTUAL_OBJECT_ROLE_NONE ||
               (self->role == role && self->role_object == NULL);

  if (!valid) {
    return false;
  }

  zn_assert(self->role_object == NULL, "Invalid role_object state");

  self->role = role;
  self->role_object = role_object;

  if (self->role_object) {
    switch (self->role) {
      case ZN_VIRTUAL_OBJECT_ROLE_NONE:
        zn_assert(false, "unreachable");
        break;
      case ZN_VIRTUAL_OBJECT_ROLE_BOUNDED:
        wl_signal_add(&self->bounded->events.destroy,
            &self->role_object_destroy_listener);
        break;
    }
  }

  return true;
}

static void
zn_virtual_object_handle_role_object_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_virtual_object *self =
      zn_container_of(listener, self, role_object_destroy_listener);

  self->role_object = NULL;
  wl_list_remove(&self->role_object_destroy_listener.link);
  wl_list_init(&self->role_object_destroy_listener.link);
}

void
zn_virtual_object_commit(struct zn_virtual_object *self)
{
  zn_signal_emit_mutable(&self->events.commit, NULL);
  self->impl->committed(self);
}

struct zn_virtual_object *
zn_virtual_object_create(
    void *impl_data, const struct zn_virtual_object_interface *implementation)
{
  struct zn_virtual_object *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->impl_data = impl_data;
  self->impl = implementation;
  wl_signal_init(&self->events.destroy);
  wl_signal_init(&self->events.commit);

  self->role_object_destroy_listener.notify =
      zn_virtual_object_handle_role_object_destroy;
  wl_list_init(&self->role_object_destroy_listener.link);

  return self;

err:
  return NULL;
}

void
zn_virtual_object_destroy(struct zn_virtual_object *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->role_object_destroy_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->events.commit.listener_list);
  free(self);
}
