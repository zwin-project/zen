#include "gl-program.h"

#include <zen-common.h>
#include <zigen-gles-v32-protocol.h>

static void zgnr_gl_program_destroy(struct zgnr_gl_program_impl* self);

static void
zgnr_gl_program_handle_destroy(struct wl_resource* resource)
{
  struct zgnr_gl_program_impl* self = wl_resource_get_user_data(resource);

  zgnr_gl_program_destroy(self);
}

static void
zgnr_gl_program_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zgnr_gl_program_protocol_attach(struct wl_client* client,
    struct wl_resource* resource, struct wl_resource* shader)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(shader);
}

static void
zgnr_gl_program_protocol_link(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  UNUSED(resource);
}

static const struct zgn_gl_program_interface implementation = {
    .destroy = zgnr_gl_program_protocol_destroy,
    .attach = zgnr_gl_program_protocol_attach,
    .link = zgnr_gl_program_protocol_link,
};

struct zgnr_gl_program_impl*
zgnr_gl_program_create(struct wl_client* client, uint32_t id)
{
  struct zgnr_gl_program_impl* self;
  struct wl_resource* resource;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  resource = wl_resource_create(client, &zgn_gl_program_interface, 1, id);
  if (resource == NULL) {
    zn_error("Failed to creat a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      resource, &implementation, self, zgnr_gl_program_handle_destroy);

  wl_signal_init(&self->base.events.destroy);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_gl_program_destroy(struct zgnr_gl_program_impl* self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  wl_list_remove(&self->base.events.destroy.listener_list);
  free(self);
}
