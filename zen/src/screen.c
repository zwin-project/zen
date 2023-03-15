#include "zen/screen.h"

#include <cglm/vec2.h>
#include <wayland-server-core.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/snode.h"

void
zn_screen_set_layout_position(struct zn_screen *self, vec2 layout_position)
{
  glm_vec2_copy(layout_position, self->layout_position);

  wl_signal_emit(&self->events.layout_position_changed, NULL);
}

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

  self->impl_data = impl_data;
  self->impl = implementation;
  self->user_data = NULL;
  glm_vec2_zero(self->size);
  wl_signal_init(&self->events.resized);
  wl_signal_init(&self->events.destroy);
  wl_signal_init(&self->events.layout_position_changed);

  self->snode_root = zn_snode_create_root(self);
  if (self->snode_root == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  for (int i = 0; i < 4; i++) {  // for each enum zwlr_layer_shell_v1_layer
    self->layers[i] = self->impl->get_layer(self->impl_data, i);
    if (self->layers[i] == NULL) {
      zn_error("Failed to get screen layer node (%d)", i);
      goto err_snode_root;
    }
  }

  return self;

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

  zn_snode_destroy(self->snode_root);
  wl_list_remove(&self->events.layout_position_changed.listener_list);
  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->events.resized.listener_list);
  free(self);
}
