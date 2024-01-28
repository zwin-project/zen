#include "zen/virtual-object.h"

#include <cglm/quat.h>
#include <cglm/vec3.h>

#include "gl-virtual-object.h"
#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/bounded.h"
#include "zen/inode.h"
#include "zen/server.h"
#include "zen/xr-dispatcher.h"
#include "zen/xr-system.h"

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
zn_virtual_object_change_visibility(
    struct zn_virtual_object *self, bool visible)
{
  if (self->gl_virtual_object) {
    zn_gl_virtual_object_change_visibility(self->gl_virtual_object, visible);
  }
}

void
zn_virtual_object_commit(struct zn_virtual_object *self)
{
  zn_signal_emit_mutable(&self->events.commit, NULL);

  if (self->gl_virtual_object) {
    zn_gl_virtual_object_committed(self->gl_virtual_object);
  }
}

static void
zn_virtual_object_inode_mapped(void *impl_data UNUSED)
{}

static void
zn_virtual_object_inode_unmapped(void *impl_data UNUSED)
{}

static void
zn_virtual_object_inode_activated(void *impl_data)
{
  struct zn_virtual_object *self = impl_data;
  struct zn_xr_system *xr_system = self->inode->xr_system;

  zn_assert(xr_system != NULL, "active inode must have xr_system");
  zn_assert(self->gl_virtual_object == NULL,
      "gl_virtual_object must be NULL before being activated");

  self->gl_virtual_object =
      zn_xr_dispatcher_get_new_gl_virtual_object(xr_system->default_dispatcher);
}

static void
zn_virtual_object_inode_deactivated(void *impl_data)
{
  struct zn_virtual_object *self = impl_data;
  self->gl_virtual_object = NULL;
}

static void
zn_virtual_object_inode_moved(void *impl_data UNUSED)
{}

static const struct zn_inode_interface inode_implementation = {
    .mapped = zn_virtual_object_inode_mapped,
    .unmapped = zn_virtual_object_inode_unmapped,
    .activated = zn_virtual_object_inode_activated,
    .deactivated = zn_virtual_object_inode_deactivated,
    .moved = zn_virtual_object_inode_moved,
};

struct zn_virtual_object *
zn_virtual_object_create(void)
{
  struct zn_server *server = zn_server_get_singleton();

  struct zn_virtual_object *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->inode = zn_inode_create(self, &inode_implementation);
  if (self->inode == NULL) {
    zn_error("Failed to create a inode");
    goto err_free;
  }

  self->gl_virtual_object = NULL;
  self->role = ZN_VIRTUAL_OBJECT_ROLE_NONE;

  wl_signal_init(&self->events.destroy);
  wl_signal_init(&self->events.commit);

  self->role_object_destroy_listener.notify =
      zn_virtual_object_handle_role_object_destroy;
  wl_list_init(&self->role_object_destroy_listener.link);

  zn_inode_move(self->inode, server->inode_invisible_root, GLM_VEC3_ZERO,
      GLM_QUAT_IDENTITY);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_virtual_object_destroy(struct zn_virtual_object *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  if (self->gl_virtual_object) {
    zn_xr_dispatcher_destroy_gl_virtual_object(
        self->gl_virtual_object->dispatcher, self->gl_virtual_object);
  }

  zn_inode_destroy(self->inode);
  wl_list_remove(&self->role_object_destroy_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->events.commit.listener_list);
  free(self);
}
