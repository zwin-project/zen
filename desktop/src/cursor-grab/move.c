#include "zen-desktop/cursor-grab/move.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/screen.h"
#include "zen-desktop/shell.h"
#include "zen-desktop/view.h"
#include "zen/cursor.h"
#include "zen/seat.h"
#include "zen/server.h"
#include "zen/snode.h"

static void zn_cursor_move_grab_destroy(struct zn_cursor_move_grab *self);

static struct zn_cursor_move_grab *
zn_cursor_move_grab_get(struct zn_cursor_grab *grab)
{
  struct zn_cursor_move_grab *self = zn_container_of(grab, self, base);
  return self;
}

static void
zn_cursor_move_grab_handle_motion_relative(
    struct zn_cursor_grab *grab, vec2 delta, uint32_t time_msec UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->seat->cursor;

  struct zn_cursor_move_grab *self = zn_cursor_move_grab_get(grab);
  vec2 new_position;

  zn_cursor_grab_move_relative(delta);

  if (cursor->snode->screen == NULL) {
    return;
  }

  struct zn_desktop_screen *screen =
      zn_desktop_screen_get(cursor->snode->screen);

  glm_vec2_add(cursor->snode->position, self->initial_view_cursor_position,
      new_position);

  zn_snode_set_position(self->view->snode, screen->view_layer, new_position);
}

static void
zn_cursor_move_grab_handle_rebase(struct zn_cursor_grab *grab UNUSED)
{}

static void
zn_cursor_move_grab_handle_button(struct zn_cursor_grab *grab UNUSED,
    uint32_t time_msec UNUSED, uint32_t button UNUSED,
    enum wlr_button_state state)
{
  if (state == WLR_BUTTON_RELEASED) {
    struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
    zn_desktop_shell_end_cursor_grab(shell);
  }
}

static void
zn_cursor_move_grab_handle_axis(struct zn_cursor_grab *grab UNUSED,
    uint32_t time_msec UNUSED, enum wlr_axis_source source UNUSED,
    enum wlr_axis_orientation orientation UNUSED, double delta UNUSED,
    int32_t delta_discrete UNUSED)
{}

static void
zn_cursor_move_grab_handle_frame(struct zn_cursor_grab *grab UNUSED)
{}

static void
zn_cursor_move_grab_handle_destroy(struct zn_cursor_grab *grab)
{
  struct zn_cursor_move_grab *self = zn_cursor_move_grab_get(grab);
  zn_cursor_move_grab_destroy(self);
}

static const struct zn_cursor_grab_interface implementation = {
    .motion_relative = zn_cursor_move_grab_handle_motion_relative,
    .rebase = zn_cursor_move_grab_handle_rebase,
    .button = zn_cursor_move_grab_handle_button,
    .axis = zn_cursor_move_grab_handle_axis,
    .frame = zn_cursor_move_grab_handle_frame,
    .destroy = zn_cursor_move_grab_handle_destroy,
};

static void
zn_cursor_move_grab_handle_view_destroy(
    struct wl_listener *listener UNUSED, void *data UNUSED)
{
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
  zn_desktop_shell_end_cursor_grab(shell);
}

static struct zn_cursor_move_grab *
zn_cursor_move_grab_create(struct zn_desktop_view *view)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->seat->cursor;

  struct zn_cursor_move_grab *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.impl = &implementation;
  self->view = view;

  glm_vec2_sub(view->snode->absolute_position, cursor->snode->absolute_position,
      self->initial_view_cursor_position);

  self->view_destroy_listener.notify = zn_cursor_move_grab_handle_view_destroy;
  wl_signal_add(&view->events.destroy, &self->view_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zn_cursor_move_grab_destroy(struct zn_cursor_move_grab *self)
{
  wl_list_remove(&self->view_destroy_listener.link);
  free(self);
}

bool
zn_cursor_move_grab_start(struct zn_desktop_view *view)
{
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
  struct zn_server *server = zn_server_get_singleton();

  struct zn_cursor_move_grab *self = zn_cursor_move_grab_create(view);
  if (self == NULL) {
    zn_error("Failed to create a move grab");
    goto err;
  }

  /// TODO(@Aki-7): close popups
  zn_seat_pointer_enter(server->seat, NULL, GLM_VEC2_ZERO);

  zn_cursor_set_xcursor_grabbing(server->seat->cursor);

  zn_desktop_shell_start_cursor_grab(shell, &self->base);

  return true;

err:
  return false;
}
