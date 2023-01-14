#include "texture-binding.h"

#include <zen-common.h>

void zwnr_texture_binding_destroy(struct zwnr_texture_binding_impl *self);

static void
zwnr_texture_binding_handle_texture_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zwnr_texture_binding_impl *self =
      zn_container_of(listener, self, texture_destroy_listener);

  zwnr_texture_binding_destroy(self);
}

static void
zwnr_texture_binding_handle_sampler_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zwnr_texture_binding_impl *self =
      zn_container_of(listener, self, sampler_destroy_listener);

  zwnr_texture_binding_destroy(self);
}

struct zwnr_texture_binding_impl *
zwnr_texture_binding_create(uint32_t binding, const char *name,
    struct zwnr_gl_texture_impl *texture, uint32_t target,
    struct zwnr_gl_sampler_impl *sampler)
{
  struct zwnr_texture_binding_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->base.binding = binding;
  self->base.name = strdup(name);
  self->base.texture = &texture->base;
  self->base.sampler = &sampler->base;
  self->base.target = target;
  wl_list_init(&self->base.link);

  self->texture_destroy_listener.notify =
      zwnr_texture_binding_handle_texture_destroy;
  wl_signal_add(&texture->base.events.destroy, &self->texture_destroy_listener);

  self->sampler_destroy_listener.notify =
      zwnr_texture_binding_handle_sampler_destroy;
  wl_signal_add(&sampler->base.events.destroy, &self->sampler_destroy_listener);

  return self;

err:
  return NULL;
}

void
zwnr_texture_binding_destroy(struct zwnr_texture_binding_impl *self)
{
  wl_list_remove(&self->texture_destroy_listener.link);
  wl_list_remove(&self->sampler_destroy_listener.link);
  wl_list_remove(&self->base.link);
  free(self->base.name);
  free(self);
}
