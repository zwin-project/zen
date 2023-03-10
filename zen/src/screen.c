#include "zen/screen.h"

#include <cglm/vec2.h>
#include <wayland-server-core.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/snode.h"

void
zn_screen_damage(struct zn_screen *self, struct wlr_fbox *fbox)
{
  self->impl->damage(self->impl_data, fbox);
}

void
zn_screen_notify_resize(struct zn_screen *self, vec2 size)
{
  glm_vec2_copy(size, self->size);
  wl_signal_emit(&self->events.resized, NULL);
}

void
zn_screen_notify_frame(struct zn_screen *self, struct timespec *when)
{
  zn_snode_notify_frame(self->snode_root, when);
}

struct zn_screen *
zn_screen_create(
    void *impl_data, const struct zn_screen_interface *implementation)
{
  struct zn_screen *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->snode_root = zn_snode_create_root(self);
  if (self->snode_root == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  self->layers.background =
      zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->layers.background == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_snode_root;
  }

  self->layers.bottom = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->layers.bottom == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_layer_background;
  }

  self->layers.top = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->layers.top == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_layer_bottom;
  }

  self->layers.overlay = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->layers.overlay == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_layer_top;
  }

  self->impl_data = impl_data;
  self->impl = implementation;
  self->user_data = NULL;
  glm_vec2_zero(self->size);
  wl_signal_init(&self->events.resized);
  wl_signal_init(&self->events.destroy);

  return self;

err_layer_top:
  zn_snode_destroy(self->layers.top);

err_layer_bottom:
  zn_snode_destroy(self->layers.bottom);

err_layer_background:
  zn_snode_destroy(self->layers.background);

err_snode_root:
  zn_snode_destroy(self->snode_root);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_screen_destroy(struct zn_screen *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  zn_snode_destroy(self->layers.overlay);
  zn_snode_destroy(self->layers.top);
  zn_snode_destroy(self->layers.bottom);
  zn_snode_destroy(self->layers.background);
  zn_snode_destroy(self->snode_root);
  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->events.resized.listener_list);
  free(self);
}
