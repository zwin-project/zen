#include "zen-desktop/view.h"

#include <cglm/vec2.h>
#include <wlr/util/edges.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen-desktop/cursor-grab/move.h"
#include "zen-desktop/cursor-grab/resize.h"
#include "zen-desktop/ui/decoration.h"
#include "zen-desktop/ui/header-bar.h"
#include "zen/snode.h"
#include "zen/view.h"

static void zn_desktop_view_destroy(struct zn_desktop_view *self);

static void
zn_desktop_view_on_focus(void *user_data, bool focused)
{
  struct zn_desktop_view *self = user_data;

  if (self->zn_view->has_focus == focused) {
    return;
  }

  zn_view_set_focus(self->zn_view, focused);

  if (focused) {
    zn_snode_move_front(self->snode);
  }
}

const struct zn_snode_interface snode_implementation = {
    .get_texture = zn_snode_noop_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = zn_snode_noop_accepts_input,
    .pointer_button = zn_snode_noop_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_snode_noop_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
    .on_focus = zn_desktop_view_on_focus,
};

static void
zn_desktop_view_update_decoration(struct zn_desktop_view *self)
{
  if (self->zn_view->decoration_mode == ZN_VIEW_DECORATION_MODE_CLIENT_SIDE) {
    zn_snode_set_position(self->decoration->snode, NULL, GLM_VEC2_ZERO);

    zn_snode_set_position(self->zn_view->snode, self->snode, GLM_VEC2_ZERO);
  } else {
    zn_snode_set_position(self->decoration->snode, self->snode, GLM_VEC2_ZERO);

    zn_ui_decoration_set_content_size(self->decoration, self->zn_view->size);
    zn_snode_set_position(
        self->zn_view->snode, self->snode, self->decoration->content_offset);
  }
}

static void
zn_desktop_view_handle_header_pressed(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_desktop_view *self =
      zn_container_of(listener, self, header_pressed_listener);

  zn_cursor_move_grab_start(self);
}

static void
zn_desktop_view_handle_zn_view_resized(struct wl_listener *listener, void *data)
{
  struct zn_desktop_view *self =
      zn_container_of(listener, self, zn_view_resized_listener);
  struct zn_view_resized_event *event = data;

  vec2 size_diff;
  vec2 position_diff = GLM_VEC2_ZERO_INIT;
  bool position_changed = false;

  glm_vec2_sub(self->zn_view->size, event->previous_size, size_diff);

  if ((self->resize_edges & WLR_EDGE_LEFT) == WLR_EDGE_LEFT) {
    position_diff[0] = -size_diff[0];
    position_changed = true;
  }

  if ((self->resize_edges & WLR_EDGE_TOP) == WLR_EDGE_TOP) {
    position_diff[1] = -size_diff[1];
    position_changed = true;
  }

  if (position_changed) {
    vec2 new_position;
    glm_vec2_add(self->snode->position, position_diff, new_position);
    zn_snode_set_position(self->snode, self->snode->parent, new_position);
  }

  zn_desktop_view_update_decoration(self);
}

static void
zn_desktop_view_handle_zn_view_decoration(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_desktop_view *self =
      zn_container_of(listener, self, zn_view_decoration_listener);

  zn_desktop_view_update_decoration(self);
}

static void
zn_desktop_view_handle_zn_view_move_request(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_desktop_view *self =
      zn_container_of(listener, self, zn_view_move_request_listener);
  zn_cursor_move_grab_start(self);
}

static void
zn_desktop_view_handle_zn_view_resize_request(
    struct wl_listener *listener, void *data)
{
  struct zn_desktop_view *self =
      zn_container_of(listener, self, zn_view_resize_request_listener);
  struct zn_view_resize_event *event = data;

  zn_cursor_resize_grab_start(self, event->edges);
}

static void
zn_desktop_view_handle_zn_view_unmap(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_desktop_view *self =
      zn_container_of(listener, self, zn_view_unmap_listener);

  zn_desktop_view_destroy(self);
}

struct zn_desktop_view *
zn_desktop_view_create(struct zn_view *zn_view)
{
  struct zn_desktop_view *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zn_view = zn_view;
  wl_signal_init(&self->events.destroy);

  self->snode = zn_snode_create_focusable(self, &snode_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  self->decoration = zn_ui_decoration_create();
  if (self->decoration == NULL) {
    zn_error("Failed to create a zn_ui_decoration");
    goto err_snode;
  }

  self->header_pressed_listener.notify = zn_desktop_view_handle_header_pressed;
  wl_signal_add(&self->decoration->header_bar->events.pressed,
      &self->header_pressed_listener);

  self->zn_view_resized_listener.notify =
      zn_desktop_view_handle_zn_view_resized;
  wl_signal_add(&zn_view->events.resized, &self->zn_view_resized_listener);

  self->zn_view_unmap_listener.notify = zn_desktop_view_handle_zn_view_unmap;
  wl_signal_add(&zn_view->events.unmap, &self->zn_view_unmap_listener);

  self->zn_view_move_request_listener.notify =
      zn_desktop_view_handle_zn_view_move_request;
  wl_signal_add(
      &zn_view->events.move_request, &self->zn_view_move_request_listener);

  self->zn_view_resize_request_listener.notify =
      zn_desktop_view_handle_zn_view_resize_request;
  wl_signal_add(
      &zn_view->events.resize_request, &self->zn_view_resize_request_listener);

  self->zn_view_decoration_listener.notify =
      zn_desktop_view_handle_zn_view_decoration;
  wl_signal_add(
      &zn_view->events.decoration, &self->zn_view_decoration_listener);

  zn_desktop_view_update_decoration(self);

  return self;

err_snode:
  zn_snode_destroy(self->snode);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_desktop_view_destroy(struct zn_desktop_view *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  zn_ui_decoration_destroy(self->decoration);
  zn_snode_destroy(self->snode);
  wl_list_remove(&self->zn_view_decoration_listener.link);
  wl_list_remove(&self->zn_view_resize_request_listener.link);
  wl_list_remove(&self->zn_view_move_request_listener.link);
  wl_list_remove(&self->zn_view_unmap_listener.link);
  wl_list_remove(&self->zn_view_resized_listener.link);
  wl_list_remove(&self->header_pressed_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
