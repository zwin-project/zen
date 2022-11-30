#include "texture-binding.h"

#include <zen-common.h>

void zgnr_texture_binding_destroy(struct zgnr_texture_binding_impl* self);

static void
zgnr_texture_binding_handle_texture_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zgnr_texture_binding_impl* self =
      zn_container_of(listener, self, texture_destroy_listener);

  zgnr_texture_binding_destroy(self);
}

struct zgnr_texture_binding_impl*
zgnr_texture_binding_create(uint32_t binding, const char* name,
    struct zgnr_gl_texture_impl* texture, uint32_t target)
{
  struct zgnr_texture_binding_impl* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->base.binding = binding;
  self->base.name = strdup(name);
  self->base.texture = &texture->base;
  self->base.target = target;
  wl_list_init(&self->base.link);

  self->texture_destroy_listener.notify =
      zgnr_texture_binding_handle_texture_destroy;
  wl_signal_add(&texture->base.events.destroy, &self->texture_destroy_listener);

  return self;

err:
  return NULL;
}

void
zgnr_texture_binding_destroy(struct zgnr_texture_binding_impl* self)
{
  wl_list_remove(&self->texture_destroy_listener.link);
  wl_list_remove(&self->base.link);
  free(self->base.name);
  free(self);
}
