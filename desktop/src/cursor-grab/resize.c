#include "zen-desktop/cursor-grab/resize.h"

#include <cglm/vec2.h>
#include <wlr/util/edges.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/shell.h"
#include "zen-desktop/view.h"
#include "zen/cursor.h"
#include "zen/seat.h"
#include "zen/server.h"
#include "zen/snode.h"
#include "zen/view.h"

static void zn_cursor_resize_grab_destroy(struct zn_cursor_resize_grab *self);

static struct zn_cursor_resize_grab *
zn_cursor_resize_grab_get(struct zn_cursor_grab *grab)
{
  struct zn_cursor_resize_grab *self = zn_container_of(grab, self, base);
  return self;
}

static void
zn_cursor_resize_grab_handle_motion_relative(
    struct zn_cursor_grab *grab, vec2 delta, uint32_t time_msec UNUSED)
{
  struct zn_cursor_resize_grab *self = zn_cursor_resize_grab_get(grab);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->seat->cursor;

  zn_cursor_grab_move_relative(delta);

  vec2 cursor_delta;
  vec2 view_delta;
  vec2 next_view_size;

  glm_vec2_sub(cursor->snode->absolute_position, self->initial_cursor_position,
      cursor_delta);
  glm_vec2_copy(cursor_delta, view_delta);

  if ((self->edges & WLR_EDGE_LEFT) == WLR_EDGE_LEFT) {
    view_delta[0] *= -1;
  } else if ((self->edges & WLR_EDGE_RIGHT) == WLR_EDGE_RIGHT) {
  } else {
    view_delta[0] *= 0;
  }

  if ((self->edges & WLR_EDGE_TOP) == WLR_EDGE_TOP) {
    view_delta[1] *= -1;
  } else if ((self->edges & WLR_EDGE_BOTTOM) == WLR_EDGE_BOTTOM) {
  } else {
    view_delta[1] *= 0;
  }

  glm_vec2_add(self->initial_view_size, view_delta, next_view_size);

  self->view->resize_edges = self->edges;

  zn_view_configure_size(self->view->zn_view, next_view_size);
}

static void
zn_cursor_resize_grab_handle_rebase(struct zn_cursor_grab *grab UNUSED)
{}

static void
zn_cursor_resize_grab_handle_button(struct zn_cursor_grab *grab UNUSED,
    uint32_t time_msec UNUSED, uint32_t button UNUSED,
    enum wlr_button_state state UNUSED)
{
  if (state == WLR_BUTTON_RELEASED) {
    struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
    zn_desktop_shell_end_cursor_grab(shell);
  }
}

static void
zn_cursor_resize_grab_handle_axis(struct zn_cursor_grab *grab UNUSED,
    uint32_t time_msec UNUSED, enum wlr_axis_source source UNUSED,
    enum wlr_axis_orientation orientation UNUSED, double delta UNUSED,
    int32_t delta_discrete UNUSED)
{}

static void
zn_cursor_resize_grab_handle_frame(struct zn_cursor_grab *grab UNUSED)
{}

static void
zn_cursor_resize_grab_handle_destroy(struct zn_cursor_grab *grab)
{
  struct zn_cursor_resize_grab *self = zn_cursor_resize_grab_get(grab);
  zn_cursor_resize_grab_destroy(self);
}

static const struct zn_cursor_grab_interface implementation = {
    .motion_relative = zn_cursor_resize_grab_handle_motion_relative,
    .rebase = zn_cursor_resize_grab_handle_rebase,
    .button = zn_cursor_resize_grab_handle_button,
    .axis = zn_cursor_resize_grab_handle_axis,
    .frame = zn_cursor_resize_grab_handle_frame,
    .destroy = zn_cursor_resize_grab_handle_destroy,
};

static void
zn_cursor_resize_grab_handle_view_destroy(
    struct wl_listener *listener UNUSED, void *data UNUSED)
{
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
  zn_desktop_shell_end_cursor_grab(shell);
}

static struct zn_cursor_resize_grab *
zn_cursor_resize_grab_create(struct zn_desktop_view *view, uint32_t edges)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->seat->cursor;

  struct zn_cursor_resize_grab *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.impl = &implementation;
  self->view = view;

  self->edges = edges;
  glm_vec2_copy(view->zn_view->size, self->initial_view_size);
  glm_vec2_copy(
      cursor->snode->absolute_position, self->initial_cursor_position);

  self->view_destroy_listener.notify =
      zn_cursor_resize_grab_handle_view_destroy;
  wl_signal_add(&view->events.destroy, &self->view_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zn_cursor_resize_grab_destroy(struct zn_cursor_resize_grab *self)
{
  wl_list_remove(&self->view_destroy_listener.link);
  free(self);
}

bool
zn_cursor_resize_grab_start(struct zn_desktop_view *view, uint32_t edges)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();

  struct zn_cursor_resize_grab *self =
      zn_cursor_resize_grab_create(view, edges);
  if (self == NULL) {
    zn_error("Failed to create a resize grab");
    goto err;
  }

  // TODO(@Aki-7): close popups
  zn_seat_pointer_enter(server->seat, NULL, GLM_VEC2_ZERO);

  zn_cursor_set_xcursor_edges(server->seat->cursor, edges);

  zn_desktop_shell_start_cursor_grab(shell, &self->base);

  return true;

err:
  return false;
}
