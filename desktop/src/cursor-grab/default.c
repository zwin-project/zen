#include "zen-desktop/cursor-grab/default.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/screen-layout.h"
#include "zen-desktop/screen.h"
#include "zen-desktop/shell.h"
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
    struct zn_cursor_grab *grab UNUSED, vec2 delta, uint32_t time_msec UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
  struct zn_cursor *cursor = server->seat->cursor;
  struct zn_desktop_screen *desktop_screen =
      cursor->snode->screen ? zn_desktop_screen_get(cursor->snode->screen)
                            : NULL;
  vec2 position;

  if (desktop_screen == NULL) {
    desktop_screen = zn_screen_layout_get_main_screen(shell->screen_layout);

    if (!desktop_screen) {
      return;
    }

    glm_vec2_divs(desktop_screen->screen->size, 2, position);
  } else {
    glm_vec2_add(cursor->snode->position, delta, position);
    zn_screen_layout_get_closest_position(shell->screen_layout, desktop_screen,
        position, &desktop_screen, position);
  }

  zn_snode_set_position(cursor->snode, desktop_screen->cursor_layer, position);

  // TODO(@Aki-7) : send events to views, etc
}

static void
zn_cursor_default_grab_handle_button(struct zn_cursor_grab *grab UNUSED,
    uint32_t time_msec UNUSED, uint32_t button UNUSED,
    enum wlr_button_state state UNUSED)
{}

static void
zn_cursor_default_grab_handle_axis(struct zn_cursor_grab *grab UNUSED,
    uint32_t time_msec UNUSED, enum wlr_axis_source source UNUSED,
    enum wlr_axis_orientation orientation UNUSED, double delta UNUSED,
    int32_t delta_discrete UNUSED)
{}

static void
zn_cursor_default_grab_handle_frame(struct zn_cursor_grab *grab UNUSED)
{}

static void
zn_cursor_default_grab_handle_destroy(struct zn_cursor_grab *grab)
{
  struct zn_cursor_default_grab *self = zn_cursor_default_grab_get(grab);
  zn_cursor_default_grab_destroy(self);
}

static const struct zn_cursor_grab_interface implementation = {
    .motion_relative = zn_cursor_default_grab_handle_motion_relative,
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
