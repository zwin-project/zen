#include "zen/scene.h"

#include <zen-common.h>

#include "zen/board.h"
#include "zen/cursor.h"
#include "zen/ray.h"
#include "zen/screen.h"
#include "zen/server.h"

static struct zn_board *
zn_scene_ensure_dangling_board(struct zn_scene *self)
{
  struct zn_board *board;
  wl_list_for_each (board, &self->board_list, link) {
    if (zn_board_is_dangling(board)) return board;
  }

  board = zn_board_create();

  wl_list_insert(&self->board_list, &board->link);

  wl_signal_emit(&self->events.new_board, board);

  return board;
}

void
zn_scene_new_screen(struct zn_scene *self, struct zn_screen *screen)
{
  wl_list_insert(&self->screen_list, &screen->link);
  struct zn_server *server = zn_server_get_singleton();

  struct zn_board *board = zn_scene_ensure_dangling_board(self);

  zn_board_set_screen(board, screen);
  screen->board = board;

  if (server->display_system == ZN_DISPLAY_SYSTEM_SCREEN &&
      wl_list_length(&self->screen_list) == 1) {
    double width, height;
    zn_board_get_effective_size(board, &width, &height);
    zn_cursor_move(server->scene->cursor, board, width / 2.f, height / 2.f);
  }
}

struct zn_scene *
zn_scene_create(void)
{
  struct zn_scene *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->ray = zn_ray_create();
  if (self->ray == NULL) {
    zn_error("Failed to create a ray");
    goto err_free;
  }

  self->cursor = zn_cursor_create();
  if (self->cursor == NULL) {
    zn_error("Failed to creat a cursor");
    goto err_ray;
  }

  wl_list_init(&self->screen_list);
  wl_list_init(&self->board_list);
  wl_signal_init(&self->events.new_board);

  return self;

err_ray:
  zn_ray_destroy(self->ray);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_scene_destroy_resources(struct zn_scene *self)
{
  struct zn_board *board, *tmp;
  wl_list_for_each_safe (board, tmp, &self->board_list, link) {
    zn_board_destroy(board);
  }
  zn_cursor_destroy_resources(self->cursor);
  zn_ray_destroy_resources(self->ray);
}

void
zn_scene_destroy(struct zn_scene *self)
{
  wl_list_remove(&self->events.new_board.listener_list);
  wl_list_remove(&self->screen_list);
  zn_cursor_destroy(self->cursor);
  zn_ray_destroy(self->ray);
  free(self);
}
