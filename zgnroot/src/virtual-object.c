#include "virtual-object.h"

#include <zen-common.h>
#include <zigen-protocol.h>

static void zgnr_virtual_object_destroy(struct zgnr_virtual_object_impl* self);

static void
zgnr_virtual_object_handle_destroy(struct wl_resource* resource)
{
  struct zgnr_virtual_object_impl* self = wl_resource_get_user_data(resource);
  zgnr_virtual_object_destroy(self);
}

static void
zgnr_virtual_object_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zgnr_virtual_object_protocol_commit(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  struct zgnr_virtual_object_impl* self = wl_resource_get_user_data(resource);

  wl_list_insert_list(&self->base.current.frame_callback_list,
      &self->pending.frame_callback_list);
  wl_list_init(&self->pending.frame_callback_list);

  self->base.committed = true;

  wl_signal_emit(&self->events.on_commit, NULL);
  wl_signal_emit(&self->base.events.committed, NULL);
}

static void
callback_handle_destroy(struct wl_resource* resource)
{
  wl_list_remove(wl_resource_get_link(resource));
}

static void
zgnr_virtual_object_protocol_frame(
    struct wl_client* client, struct wl_resource* resource, uint32_t callback)
{
  struct zgnr_virtual_object_impl* self = wl_resource_get_user_data(resource);
  struct wl_resource* callback_resource =
      wl_resource_create(client, &wl_callback_interface, 1, callback);
  if (callback_resource == NULL) {
    wl_resource_post_no_memory(resource);
    zn_error("Failed to create a wl_resource");
    return;
  }

  wl_resource_set_implementation(
      callback_resource, NULL, NULL, callback_handle_destroy);

  wl_list_insert(self->pending.frame_callback_list.prev,
      wl_resource_get_link(callback_resource));
}

static const struct zgn_virtual_object_interface implementation = {
    .destroy = zgnr_virtual_object_protocol_destroy,
    .commit = zgnr_virtual_object_protocol_commit,
    .frame = zgnr_virtual_object_protocol_frame,
};

void
zgnr_virtual_object_send_frame_done(
    struct zgnr_virtual_object* self, const struct timespec* when)
{
  struct wl_resource *resource, *tmp;
  wl_resource_for_each_safe (
      resource, tmp, &self->current.frame_callback_list) {
    wl_callback_send_done(resource, timespec_to_msec(when));
    wl_resource_destroy(resource);
  }
}

bool
zgnr_virtual_object_set_role(struct zgnr_virtual_object_impl* self,
    enum zgnr_virtual_object_role role, void* role_object,
    struct wl_resource* error_resource, uint32_t error_code)
{
  if (self->base.role != ZGNR_VIRTUAL_OBJECT_ROLE_NONE &&
      self->base.role != role) {
    if (error_resource != NULL) {
      wl_resource_post_error(error_resource, error_code,
          "zgn_virtual_object@%u already has another role",
          wl_resource_get_id(self->resource));
    }
    return false;
  }

  if (self->base.role_object != NULL && self->base.role_object != role_object) {
    wl_resource_post_error(error_resource, error_code,
        "zgn_virtual_object@%u already has a active role",
        wl_resource_get_id(self->resource));
    return false;
  }

  self->base.role = role;
  self->base.role_object = role_object;

  return true;
}

struct zgnr_virtual_object_impl*
zgnr_virtual_object_create(
    struct wl_client* client, uint32_t id, struct wl_display* display)
{
  struct zgnr_virtual_object_impl* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed allocate memory");
    goto err;
  }

  self->resource =
      wl_resource_create(client, &zgn_virtual_object_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zgnr_virtual_object_handle_destroy);

  wl_signal_init(&self->base.events.destroy);
  wl_signal_init(&self->base.events.committed);
  wl_list_init(&self->base.current.rendering_unit_list);
  wl_list_init(&self->base.current.frame_callback_list);
  wl_signal_init(&self->events.on_commit);
  wl_list_init(&self->pending.frame_callback_list);

  self->base.committed = false;
  self->display = display;
  self->base.role = ZGNR_VIRTUAL_OBJECT_ROLE_NONE;
  self->base.role_object = NULL;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_virtual_object_destroy(struct zgnr_virtual_object_impl* self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);
  wl_list_remove(&self->base.events.destroy.listener_list);
  wl_list_remove(&self->base.current.rendering_unit_list);
  wl_list_remove(&self->base.events.committed.listener_list);
  wl_list_remove(&self->base.current.frame_callback_list);
  wl_list_remove(&self->events.on_commit.listener_list);
  wl_list_remove(&self->pending.frame_callback_list);
  free(self);
}
