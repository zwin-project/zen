#include "gl-shader.h"

#include <zen-common.h>
#include <zwin-gles-v32-protocol.h>

#include "zwnr/shm.h"

static void zwnr_gl_shader_destroy(struct zwnr_gl_shader_impl *self);
static void zwnr_gl_shader_inert(struct zwnr_gl_shader_impl *self);

static void
zwnr_gl_shader_handle_destroy(struct wl_resource *resource)
{
  struct zwnr_gl_shader_impl *self = wl_resource_get_user_data(resource);

  zwnr_gl_shader_destroy(self);
}

static void
zwnr_gl_shader_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);

  wl_resource_destroy(resource);
}

/** Be careful, resource can be inert. */
static const struct zwn_gl_shader_interface implementation = {
    .destroy = zwnr_gl_shader_protocol_destroy,
};

static void
zwnr_gl_shader_handle_buffer_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zwnr_gl_shader_impl *self =
      zn_container_of(listener, self, buffer_destroy_listener);

  zwnr_gl_shader_inert(self);
}

static void
zwnr_gl_shader_inert(struct zwnr_gl_shader_impl *self)
{
  struct wl_resource *resource = self->resource;
  zwnr_gl_shader_destroy(self);
  wl_resource_set_user_data(resource, NULL);
  wl_resource_set_destructor(resource, NULL);
}

struct zwnr_gl_shader_impl *
zwnr_gl_shader_create(struct wl_client *client, uint32_t id,
    struct wl_resource *buffer, uint32_t type)
{
  struct zwnr_gl_shader_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->resource = wl_resource_create(client, &zwn_gl_shader_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zwnr_gl_shader_handle_destroy);

  wl_signal_init(&self->base.events.destroy);
  self->base.type = type;
  self->base.buffer = zwnr_shm_buffer_get(buffer);

  self->buffer_destroy_listener.notify = zwnr_gl_shader_handle_buffer_destroy;
  wl_resource_add_destroy_listener(buffer, &self->buffer_destroy_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zwnr_gl_shader_destroy(struct zwnr_gl_shader_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  wl_list_remove(&self->buffer_destroy_listener.link);
  wl_list_remove(&self->base.events.destroy.listener_list);
  free(self);
}
