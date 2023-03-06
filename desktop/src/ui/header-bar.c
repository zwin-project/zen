#include "zen-desktop/ui/header-bar.h"

#include <cglm/vec2.h>
#include <zen-common/log.h>
#include <zen-common/util.h>

#include "zen/snode.h"

static const struct zn_snode_interface snode_implementation = {
    .get_texture = zn_snode_noop_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = zn_snode_noop_accepts_input,
    .pointer_button = zn_snode_noop_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_snode_noop_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
};

void
zn_ui_header_bar_set_size(struct zn_ui_header_bar *self, vec2 size)
{
  glm_vec2_copy(size, self->size);
}

struct zn_ui_header_bar *
zn_ui_header_bar_create(void)
{
  struct zn_ui_header_bar *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->snode = zn_snode_create(self, &snode_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a snode");
    goto err_free;
  }

  glm_vec2_copy(GLM_VEC2_ZERO, self->size);
  wl_signal_init(&self->events.move);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ui_header_bar_destroy(struct zn_ui_header_bar *self)
{
  wl_list_remove(&self->events.move.listener_list);
  zn_snode_destroy(self->snode);
  free(self);
}
