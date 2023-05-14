#include "bounded.h"

#include <cglm/vec3.h>
#include <zwin-protocol.h>
#include <zwin-shell-protocol.h>

#include "backend/default-backend.h"
#include "bounded-configure.h"
#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen-common/wl-array.h"
#include "zen/server.h"
#include "zen/virtual-object.h"

static void zn_bounded_destroy(struct zn_bounded *self);

static void
zn_bounded_send_configure(void *user_data)
{
  struct zn_bounded *self = user_data;

  self->scheduled_config.idle = NULL;

  struct wl_array half_size_array;
  wl_array_init(&half_size_array);

  if (!zn_wl_array_from_vec3(
          &half_size_array, self->scheduled_config.half_size)) {
    zn_error("Failed to convert from vec3 to wl_array");
    goto out;
  }

  struct zn_bounded_configure *configure = zn_bounded_configure_create(
      self->scheduled_config.serial, self->scheduled_config.half_size);
  if (configure == NULL) {
    wl_client_post_no_memory(wl_resource_get_client(self->resource));
    zn_error("Failed to create a zn_bounded_configure");
    goto out;
  }

  zwn_bounded_send_configure(
      self->resource, &half_size_array, self->scheduled_config.serial);

  wl_list_insert(self->configure_list.prev, &configure->link);

out:
  wl_array_release(&half_size_array);
}

uint32_t
zn_bounded_schedule_configure(struct zn_bounded *self)
{
  struct wl_client *client = wl_resource_get_client(self->resource);
  struct wl_display *display = wl_client_get_display(client);
  struct wl_event_loop *loop = wl_display_get_event_loop(display);

  if (self->scheduled_config.idle == NULL) {
    self->scheduled_config.serial = wl_display_get_serial(display);
    self->scheduled_config.idle =
        wl_event_loop_add_idle(loop, zn_bounded_send_configure, self);
    if (self->scheduled_config.idle == NULL) {
      wl_client_post_no_memory(client);
    }
  }

  return self->scheduled_config.serial;
}

static void
zn_bounded_handle_destroy(struct wl_resource *resource)
{
  struct zn_bounded *self = wl_resource_get_user_data(resource);

  zn_bounded_destroy(self);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_bounded_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_bounded_protocol_ack_configure(struct wl_client *client UNUSED,
    struct wl_resource *resource, struct wl_array *half_size_array,
    uint32_t serial)
{
  struct zn_bounded *self = wl_resource_get_user_data(resource);
  if (self == NULL) {
    return;
  }

  vec3 half_size;

  if (!zn_wl_array_to_vec3(half_size_array, half_size)) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_INVALID_WL_ARRAY_SIZE,
        "invalid wl_array size");
    return;
  }

  bool found = false;
  struct zn_bounded_configure *configure = NULL;

  wl_list_for_each (configure, &self->configure_list, link) {
    if (configure->serial == serial) {
      found = true;
      break;
    }
  }

  if (!found) {
    wl_resource_post_error(resource, ZWN_BOUNDED_ERROR_INVALID_SERIAL,
        "wrong configure serial: %u", serial);
    return;
  }

  struct zn_bounded_configure *configure_tmp = NULL;

  wl_list_for_each_safe (
      configure, configure_tmp, &self->configure_list, link) {
    if (configure->serial == serial) {
      break;
    }

    zn_bounded_configure_destroy(configure);
  }

  self->configured = true;

  glm_vec3_copy(half_size, self->pending.half_size);

  zn_bounded_configure_destroy(configure);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_bounded_protocol_set_title(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, const char *title UNUSED)
{}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_bounded_protocol_set_input_region(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, struct wl_resource *region UNUSED)
{}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_bounded_protocol_move(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, struct wl_resource *seat UNUSED,
    uint32_t serial UNUSED)
{}

static const struct zwn_bounded_interface implementation = {
    .destroy = zn_bounded_protocol_destroy,
    .ack_configure = zn_bounded_protocol_ack_configure,
    .set_title = zn_bounded_protocol_set_title,
    .set_input_region = zn_bounded_protocol_set_input_region,
    .move = zn_bounded_protocol_move,
};

static void
zn_bounded_handle_virtual_object_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_bounded *self =
      zn_container_of(listener, self, virtual_object_destroy_listener);

  zn_bounded_destroy(self);
}

static void
zn_bounded_handle_commit(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_bounded *self = zn_container_of(listener, self, commit_listener);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_default_backend *backend = zn_default_backend_get(server->backend);

  glm_vec3_copy(self->pending.half_size, self->current.half_size);

  if (!self->added) {
    // on the initial commit
    self->added = true;
    glm_vec3_zero(self->scheduled_config.half_size);
    zn_bounded_schedule_configure(self);
  }

  if (self->configured && !self->mapped) {
    self->mapped = true;
    zn_default_backend_notify_bounded_mapped(backend, self);
  }
}

struct zn_bounded *
zn_bounded_create(struct wl_client *client, uint32_t id,
    struct zn_virtual_object *virtual_object)
{
  struct zn_bounded *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_client_post_no_memory(client);
    goto err;
  }

  self->virtual_object = virtual_object;
  wl_list_init(&self->configure_list);
  glm_vec3_zero(self->scheduled_config.half_size);
  self->scheduled_config.idle = NULL;
  self->added = false;
  self->configured = false;
  self->mapped = false;
  glm_vec3_zero(self->pending.half_size);
  glm_vec3_zero(self->current.half_size);
  wl_signal_init(&self->events.destroy);

  self->resource = wl_resource_create(client, &zwn_bounded_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_free;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zn_bounded_handle_destroy);

  self->virtual_object_destroy_listener.notify =
      zn_bounded_handle_virtual_object_destroy;
  wl_signal_add(&self->virtual_object->events.destroy,
      &self->virtual_object_destroy_listener);

  self->commit_listener.notify = zn_bounded_handle_commit;
  wl_signal_add(&self->virtual_object->events.commit, &self->commit_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_bounded_destroy(struct zn_bounded *self)
{
  struct zn_bounded_configure *configure = NULL;
  struct zn_bounded_configure *configure_tmp = NULL;

  zn_signal_emit_mutable(&self->events.destroy, NULL);

  if (self->scheduled_config.idle != NULL) {
    wl_event_source_remove(self->scheduled_config.idle);
  }

  wl_list_for_each_safe (
      configure, configure_tmp, &self->configure_list, link) {
    zn_bounded_configure_destroy(configure);
  }

  wl_list_remove(&self->configure_list);
  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->virtual_object_destroy_listener.link);
  wl_list_remove(&self->commit_listener.link);
  wl_resource_set_implementation(self->resource, &implementation, NULL, NULL);
  free(self);
}
