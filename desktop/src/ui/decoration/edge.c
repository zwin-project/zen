#include "zen-desktop/ui/decoration/edge.h"

#include <cglm/vec2.h>
#include <wlr/util/edges.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-common/wlr/box.h"
#include "zen/snode.h"

#define EDGE_WIDTH 8.F

bool
zn_ui_decoration_edge_accepts_input(void *user_data, const vec2 point)
{
  struct zn_ui_decoration_edge *self = user_data;

  struct wlr_fbox content_box = {
      .x = 0,
      .y = 0,
      .width = self->size[0],
      .height = self->size[1],
  };

  struct wlr_fbox edge_box = {
      .x = -EDGE_WIDTH,
      .y = -EDGE_WIDTH,
      .width = self->size[0] + 2 * EDGE_WIDTH,
      .height = self->size[1] + 2 * EDGE_WIDTH,
  };

  if (zn_wlr_fbox_contains_point(&content_box, point[0], point[1])) {
    return false;
  }

  return zn_wlr_fbox_contains_point(&edge_box, point[0], point[1]);
}

void
zn_ui_decoration_edge_pointer_motion(
    void *user_data, uint32_t time_msec UNUSED, const vec2 point)
{
  struct zn_ui_decoration_edge *self = user_data;

  uint32_t vertical_edge = WLR_EDGE_NONE;
  uint32_t horizontal_edge = WLR_EDGE_NONE;

  if (point[0] < 0) {
    horizontal_edge = WLR_EDGE_LEFT;
  } else if (self->size[0] < point[0]) {
    horizontal_edge = WLR_EDGE_RIGHT;
  }

  if (point[1] < 0) {
    vertical_edge = WLR_EDGE_TOP;
  } else if (self->size[1] < point[1]) {
    vertical_edge = WLR_EDGE_BOTTOM;
  }

  self->edges = vertical_edge | horizontal_edge;

  if (self->edges != WLR_EDGE_NONE) {
    struct zn_ui_decoration_edge_hover_event event;
    event.edges = self->edges;
    wl_signal_emit(&self->events.hover, &event);
  }
}

void
zn_ui_decoration_edge_pointer_button(void *user_data UNUSED,
    uint32_t time_msec UNUSED, uint32_t button UNUSED,
    enum wlr_button_state state UNUSED)
{
  struct zn_ui_decoration_edge *self = user_data;

  if (self->edges != WLR_EDGE_NONE) {
    struct zn_ui_decoration_edge_pressed_event event;
    event.edges = self->edges;
    wl_signal_emit(&self->events.pressed, &event);
  }
}

const struct zn_snode_interface implementation = {
    .get_texture = zn_snode_noop_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = zn_ui_decoration_edge_accepts_input,
    .pointer_button = zn_ui_decoration_edge_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_ui_decoration_edge_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
    .on_focus = zn_snode_noop_on_focus,
};

void
zn_ui_decoration_edge_set_size(struct zn_ui_decoration_edge *self, vec2 size)
{
  glm_vec2_copy(size, self->size);
}

struct zn_ui_decoration_edge *
zn_ui_decoration_edge_create(void)
{
  struct zn_ui_decoration_edge *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->snode = zn_snode_create(self, &implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a snode");
    goto err_free;
  }

  glm_vec2_zero(self->size);
  self->edges = WLR_EDGE_NONE;

  wl_signal_init(&self->events.pressed);
  wl_signal_init(&self->events.hover);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ui_decoration_edge_destroy(struct zn_ui_decoration_edge *self)
{
  wl_list_remove(&self->events.hover.listener_list);
  wl_list_remove(&self->events.pressed.listener_list);
  zn_snode_destroy(self->snode);
  free(self);
}
