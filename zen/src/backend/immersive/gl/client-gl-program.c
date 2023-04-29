#include "client-gl-program.h"

#include <zwin-gl-protocol.h>

#include "client-gl-shader.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend.h"
#include "zen/gl-program.h"
#include "zen/server.h"
#include "zen/xr-dispatcher.h"
#include "zen/xr-system.h"

static void zn_client_gl_program_destroy(struct zn_client_gl_program *self);

static void
zn_client_gl_program_handle_destroy(struct wl_resource *resource)
{
  struct zn_client_gl_program *self = zn_client_gl_program_get(resource);

  zn_client_gl_program_destroy(self);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_program_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_program_protocol_attach_shader(struct wl_client *client UNUSED,
    struct wl_resource *resource, struct wl_resource *shader_resource)
{
  struct zn_client_gl_program *self = zn_client_gl_program_get(resource);
  struct zn_client_gl_shader *shader = zn_client_gl_shader_get(shader_resource);

  if (self == NULL || shader == NULL) {
    return;
  }

  zn_gl_program_attach_shader(self->zn_gl_program, shader->zn_gl_shader);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_program_protocol_link(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  struct zn_client_gl_program *self = zn_client_gl_program_get(resource);

  if (self == NULL) {
    return;
  }

  zn_gl_program_link(self->zn_gl_program);
}

static const struct zwn_gl_program_interface implementation = {
    .destroy = zn_client_gl_program_protocol_destroy,
    .attach_shader = zn_client_gl_program_protocol_attach_shader,
    .link = zn_client_gl_program_protocol_link,
};

struct zn_client_gl_program *
zn_client_gl_program_get(struct wl_resource *resource)
{
  return wl_resource_get_user_data(resource);
}

static void
zn_client_gl_program_handle_zn_gl_program_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_client_gl_program *self =
      zn_container_of(listener, self, zn_gl_program_destroy_listener);

  zn_client_gl_program_destroy(self);
}

struct zn_client_gl_program *
zn_client_gl_program_create(struct wl_client *client, uint32_t id)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_xr_system *xr_system = zn_backend_get_xr_system(server->backend);

  if (xr_system == NULL || !zn_xr_system_is_connected(xr_system)) {
    zn_error("Failed to get xr_system");
    goto err;
  }

  struct zn_client_gl_program *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_client_post_no_memory(client);
    goto err;
  }

  self->zn_gl_program =
      zn_xr_dispatcher_get_new_gl_program(xr_system->default_dispatcher);
  if (self->zn_gl_program == NULL) {
    zn_error("Failed to get new gl_program");
    wl_client_post_no_memory(client);
    goto err_free;
  }

  self->resource = wl_resource_create(client, &zwn_gl_program_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_gl_program;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zn_client_gl_program_handle_destroy);

  self->zn_gl_program_destroy_listener.notify =
      zn_client_gl_program_handle_zn_gl_program_destroy;
  wl_signal_add(&self->zn_gl_program->events.destroy,
      &self->zn_gl_program_destroy_listener);

  return self;

err_gl_program:
  zn_xr_dispatcher_destroy_gl_program(
      xr_system->default_dispatcher, self->zn_gl_program);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_client_gl_program_destroy(struct zn_client_gl_program *self)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_xr_system *xr_system = zn_backend_get_xr_system(server->backend);

  wl_resource_set_implementation(self->resource, &implementation, NULL, NULL);
  wl_list_remove(&self->zn_gl_program_destroy_listener.link);

  if (xr_system != NULL && zn_xr_system_is_connected(xr_system)) {
    zn_xr_dispatcher_destroy_gl_program(
        xr_system->default_dispatcher, self->zn_gl_program);
  }

  free(self);
}
