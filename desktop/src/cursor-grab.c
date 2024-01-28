#include "zen-desktop/cursor-grab.h"

#include <cglm/vec2.h>

#include "zen-desktop/screen-layout.h"
#include "zen-desktop/screen.h"
#include "zen-desktop/shell.h"
#include "zen/cursor.h"
#include "zen/screen.h"
#include "zen/seat.h"
#include "zen/server.h"
#include "zen/snode.h"

void
zn_cursor_grab_move_relative(vec2 delta)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
  struct zn_cursor *cursor = server->seat->cursor;
  struct zn_screen *screen = zn_screen_from_snode_root(cursor->snode->root);

  struct zn_desktop_screen *desktop_screen =
      screen ? zn_desktop_screen_get(screen) : NULL;

  if (desktop_screen == NULL) {
    return;
  }

  vec2 position;

  glm_vec2_add(cursor->snode->position, delta, position);
  zn_screen_layout_get_closest_position(shell->screen_layout, desktop_screen,
      position, &desktop_screen, position);

  zn_snode_set_position(cursor->snode, desktop_screen->cursor_layer, position);
}
