#include "zen-desktop/cursor-grab/default.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/screen-layout.h"
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
  struct zn_screen *screen = cursor->snode->screen;
  vec2 position;

  if (!screen) {
    screen = zn_screen_layout_get_main_screen(shell->screen_layout);

    if (!screen) {
      return;
    }

    glm_vec2_divs(screen->size, 2, position);
  } else {
    glm_vec2_add(cursor->snode->position, delta, position);
    zn_screen_layout_get_closest_position(
        shell->screen_layout, screen, position, &screen, position);
  }

  zn_snode_set_position(cursor->snode, screen->snode_root, position);

  // TODO(@Aki-7) : send events to views, etc
}

static void
zn_cursor_default_grab_handle_destroy(struct zn_cursor_grab *grab)
{
  struct zn_cursor_default_grab *self = zn_cursor_default_grab_get(grab);
  zn_cursor_default_grab_destroy(self);
}

static const struct zn_cursor_grab_interface implementation = {
    .motion_relative = zn_cursor_default_grab_handle_motion_relative,
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