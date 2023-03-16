#include "zen-desktop/cursor-grab/default.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/cursor-grab/down.h"
#include "zen/cursor.h"
#include "zen/screen.h"
#include "zen/seat.h"
#include "zen/server.h"
#include "zen/snode.h"

static void zn_cursor_default_grab_destroy(struct zn_cursor_default_grab *self);

static struct zn_cursor_default_grab *
zn_cursor_default_grab_get(struct zn_cursor_grab *grab)
{
  struct zn_cursor_default_grab *self = zn_container_of(grab, self, base);
  return self;
}

static void
zn_cursor_default_grab_handle_motion_relative(
    struct zn_cursor_grab *grab UNUSED, vec2 delta, uint32_t time_msec)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->seat->cursor;

  zn_cursor_grab_move_relative(delta);

  if (cursor->snode->screen == NULL) {
    return;
  }

  vec2 point = GLM_VEC2_ZERO_INIT;
  struct zn_snode *snode = zn_snode_get_snode_at(
      cursor->snode->screen->snode_root, cursor->snode->position, point);

  zn_seat_pointer_enter(server->seat, snode, point);

  zn_seat_pointer_motion(server->seat, time_msec, point);
}

static void
zn_cursor_default_grab_handle_rebase(struct zn_cursor_grab *grab UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->seat->cursor;

  if (cursor->snode->screen == NULL) {
    return;
  }

  vec2 point = GLM_VEC2_ZERO_INIT;
  struct zn_snode *snode = zn_snode_get_snode_at(
      cursor->snode->screen->snode_root, cursor->snode->position, point);

  zn_seat_pointer_enter(server->seat, snode, point);
}

static void
zn_cursor_default_grab_handle_button(struct zn_cursor_grab *grab UNUSED,
    uint32_t time_msec, uint32_t button, enum wlr_button_state state)
{
  struct zn_server *server = zn_server_get_singleton();

  if (state == WLR_BUTTON_PRESSED) {
    struct zn_snode *focus = NULL;
    if (server->seat->pointer_state.focus) {
      focus = zn_snode_get_focusable_parent(server->seat->pointer_state.focus);
    }

    zn_seat_set_focus(server->seat, focus);

    if (server->seat->pointer_state.focus) {
      zn_cursor_down_grab_start();
    }
  }

  zn_seat_pointer_button(server->seat, time_msec, button, state);
}

static void
zn_cursor_default_grab_handle_axis(struct zn_cursor_grab *grab UNUSED,
    uint32_t time_msec, enum wlr_axis_source source,
    enum wlr_axis_orientation orientation, double delta, int32_t delta_discrete)
{
  struct zn_server *server = zn_server_get_singleton();
  zn_seat_pointer_axis(
      server->seat, time_msec, source, orientation, delta, delta_discrete);
}

static void
zn_cursor_default_grab_handle_frame(struct zn_cursor_grab *grab UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();
  zn_seat_pointer_frame(server->seat);
}

static void
zn_cursor_default_grab_handle_destroy(struct zn_cursor_grab *grab)
{
  struct zn_cursor_default_grab *self = zn_cursor_default_grab_get(grab);
  zn_cursor_default_grab_destroy(self);
}

static const struct zn_cursor_grab_interface implementation = {
    .motion_relative = zn_cursor_default_grab_handle_motion_relative,
    .rebase = zn_cursor_default_grab_handle_rebase,
    .button = zn_cursor_default_grab_handle_button,
    .axis = zn_cursor_default_grab_handle_axis,
    .frame = zn_cursor_default_grab_handle_frame,
    .destroy = zn_cursor_default_grab_handle_destroy,
};

struct zn_cursor_default_grab *
zn_cursor_default_grab_create(void)
{
  struct zn_cursor_default_grab *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.impl = &implementation;

  return self;

err:
  return NULL;
}

static void
zn_cursor_default_grab_destroy(struct zn_cursor_default_grab *self)
{
  free(self);
}
