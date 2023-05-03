#include "bounded.h"

#include <cglm/vec3.h>
#include <zwin-protocol.h>
#include <zwin-shell-protocol.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen-common/wl-array.h"
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
    wl_array_release(&half_size_array);
    return;
  }

  zwn_bounded_send_configure(
      self->resource, &half_size_array, self->scheduled_config.serial);

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

static void
zn_bounded_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

static void
zn_bounded_protocol_ack_configure(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, struct wl_array *half_size UNUSED,
    uint32_t serial UNUSED)
{}

static void
zn_bounded_protocol_set_title(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, const char *title UNUSED)
{}

static void
zn_bounded_protocol_set_input_region(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, struct wl_resource *region UNUSED)
{}

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

  wl_resource_post_error(self->resource,
      ZWN_VIRTUAL_OBJECT_ERROR_DEFUNCT_ROLE_OBJECT,
      "Virtual object is destroyed before its role object (zwn_bounded)");

  wl_resource_destroy(self->resource);
}

static void
zn_bounded_handle_commit(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_bounded *self = zn_container_of(listener, self, commit_listener);

  if (!self->added) {
    // on the initial commit

    self->added = true;
    glm_vec3_zero(self->scheduled_config.half_size);
    zn_bounded_schedule_configure(self);
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
  wl_signal_init(&self->events.destroy);
  glm_vec3_zero(self->scheduled_config.half_size);
  self->scheduled_config.idle = NULL;
  self->added = false;

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
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  if (self->scheduled_config.idle != NULL) {
    wl_event_source_remove(self->scheduled_config.idle);
  }

  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->virtual_object_destroy_listener.link);
  wl_list_remove(&self->commit_listener.link);
  free(self);
}
