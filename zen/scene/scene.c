#include "zen/scene/scene.h"

#include <linux/input.h>

#include "zen-common.h"
#include "zen/scene/board.h"
#include "zen/scene/screen-layout.h"
#include "zen/scene/screen.h"
#include "zen/scene/view.h"

static void
zn_scene_unmap_focused_view_handler(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_scene* self =
      zn_container_of(listener, self, unmap_focused_view_listener);

  zn_scene_set_focused_view(self, NULL);
}

/**
 * @return struct zn_screen* can be NULL
 */
static struct zn_screen*
zn_scene_get_focus_screen(struct zn_scene* self)
{
  struct zn_server* server = zn_server_get_singleton();
  UNUSED(self);

  return server->input_manager->seat->cursor->screen;
}

static void
zn_scene_move_board_binding_handler(
    uint32_t time_msec, uint32_t key, void* data)
{
  struct zn_scene* scene = data;
  struct zn_screen* screen = zn_scene_get_focus_screen(scene);
  UNUSED(time_msec);

  if (screen == NULL) return;

  if (key == KEY_RIGHT) {
    zn_screen_switch_to_next_board(screen);
  } else if (key == KEY_LEFT) {
    zn_screen_switch_to_prev_board(screen);
  }
}

static void
zn_scene_new_board_binding_handler(uint32_t time_msec, uint32_t key, void* data)
{
  struct zn_scene* self = data;
  struct zn_screen* screen = zn_scene_get_focus_screen(self);
  struct zn_board* board;
  UNUSED(time_msec);
  UNUSED(key);

  if (screen == NULL) return;

  board = zn_board_create(self);
  if (board == NULL) {
    zn_error("Failed to creaet a new board");
    return;
  }

  zn_board_assign_to_screen(board, screen);
  zn_screen_set_current_board(screen, board);
}

void
zn_scene_set_focused_view(struct zn_scene* self, struct zn_view* view)
{
  if (view == self->focused_view) {
    return;
  }

  if (self->focused_view != NULL) {
    self->focused_view->impl->set_activated(self->focused_view, false);
    wl_list_remove(&self->unmap_focused_view_listener.link);
    wl_list_init(&self->unmap_focused_view_listener.link);
  }

  if (view != NULL) {
    view->impl->set_activated(view, true);
    wl_signal_add(&view->events.unmap, &self->unmap_focused_view_listener);
  }

  self->focused_view = view;
}

struct zn_board*
zn_scene_get_focus_board(struct zn_scene* self)
{
  struct zn_screen* screen = zn_scene_get_focus_screen(self);

  if (screen == NULL) {
    return NULL;
  }

  return zn_screen_get_current_board(screen);
}

static struct zn_board*
zn_scene_ensure_dangling_board(struct zn_scene* self)
{
  struct zn_board* board;
  wl_list_for_each(board, &self->board_list, link)
  {
    if (zn_board_is_dangling(board)) {
      return board;
    }
  }

  return zn_board_create(self);
}

/** Keep this idempotent. */
void
zn_scene_reassign_boards(struct zn_scene* self)
{
  struct zn_screen* screen;
  struct zn_board* board;

  // assign a board to a screen without board
  wl_list_for_each(screen, &self->screen_layout->screens, link)
  {
    struct zn_board* board;
    if (!wl_list_empty(&screen->board_list)) {
      continue;
    }

    board = zn_scene_ensure_dangling_board(self);
    if (board == NULL) {
      zn_error("Failed to create a board");
      continue;
    }

    zn_board_assign_to_screen(board, screen);
    zn_screen_set_current_board(screen, board);
  }

  // assign dangling board to a screen
  screen = NULL;
  if (!wl_list_empty(&self->screen_layout->screens)) {
    screen = zn_container_of(self->screen_layout->screens.next, screen, link);
  }

  if (screen) {
    wl_list_for_each(board, &self->board_list, link)
    {
      if (zn_board_is_dangling(board)) {
        zn_board_assign_to_screen(board, screen);
      }
    }
  }
}

void
zn_scene_setup_bindings(struct zn_scene* self)
{
  struct zn_server* server = zn_server_get_singleton();
  zn_input_manager_add_key_binding(server->input_manager, KEY_RIGHT,
      WLR_MODIFIER_SHIFT | WLR_MODIFIER_LOGO,
      zn_scene_move_board_binding_handler, self);

  zn_input_manager_add_key_binding(server->input_manager, KEY_LEFT,
      WLR_MODIFIER_SHIFT | WLR_MODIFIER_LOGO,
      zn_scene_move_board_binding_handler, self);

  zn_input_manager_add_key_binding(server->input_manager, KEY_N,
      WLR_MODIFIER_LOGO | WLR_MODIFIER_SHIFT,
      zn_scene_new_board_binding_handler, self);
}

struct zn_scene*
zn_scene_create(void)
{
  struct zn_scene* self;
  struct zn_board* board;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->screen_layout = zn_screen_layout_create(self);
  if (self->screen_layout == NULL) {
    zn_error("Failed to create zn_screen_layout");
    goto err_free;
  }

  wl_list_init(&self->board_list);

  board = zn_board_create(self);
  if (board == NULL) {
    zn_error("Failed to create a initial board");
    goto err_screen_layout;
  }

  self->unmap_focused_view_listener.notify =
      zn_scene_unmap_focused_view_handler;

  return self;

err_screen_layout:
  zn_screen_layout_destroy(self->screen_layout);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_scene_destroy(struct zn_scene* self)
{
  struct zn_board *board, *tmp;

  wl_list_for_each_safe(board, tmp, &self->board_list, link)
  {
    zn_board_destroy(board);
  }

  zn_screen_layout_destroy(self->screen_layout);

  wl_list_remove(&self->unmap_focused_view_listener.link);

  free(self);
}
