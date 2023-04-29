#include "backend/immersive/xr-gl.h"

#include <zwin-gl-protocol.h>

#include "client-gl-context.h"
#include "zen-common/log.h"
#include "zen-common/util.h"

static void
zn_xr_gl_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

static void
zn_xr_gl_protocol_create_gl_context(struct wl_client *client,
    struct wl_resource *resource UNUSED, uint32_t id,
    struct wl_resource *zwn_compositor)
{
  struct zn_xr_compositor *xr_compositor =
      wl_resource_get_user_data(zwn_compositor);

  zn_client_gl_context_create(client, id, xr_compositor);
}

static const struct zwn_gl_interface implementation = {
    .destroy = zn_xr_gl_protocol_destroy,
    .create_gl_context = zn_xr_gl_protocol_create_gl_context,
};

static void
zn_xr_gl_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zn_xr_gl *self = data;

  struct wl_resource *resource =
      wl_resource_create(client, &zwn_gl_interface, (int)version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

struct zn_xr_gl *
zn_xr_gl_create(struct wl_display *display)
{
  struct zn_xr_gl *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->global =
      wl_global_create(display, &zwn_gl_interface, 1, self, zn_xr_gl_bind);
  if (self->global == NULL) {
    zn_error("Failed to create zwn_gl global");
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_xr_gl_destroy(struct zn_xr_gl *self)
{
  wl_global_destroy(self->global);
  free(self);
}
