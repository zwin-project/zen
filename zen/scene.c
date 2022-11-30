#include "zen/scene.h"

#include <zen-common.h>

#include "zen/board.h"
#include "zen/screen.h"

static struct zn_board *
zn_scene_ensure_dangling_board(struct zn_scene *self)
{
  struct zn_board *board;
  wl_list_for_each (board, &self->board_list, link) {
    if (zn_board_is_dangling(board)) return board;
  }

  board = zn_board_create();

  wl_list_insert(&self->board_list, &board->link);

  return board;
}

void
zn_scene_new_screen(struct zn_scene *self, struct zn_screen *screen)
{
  wl_list_insert(&self->screen_list, &screen->link);

  struct zn_board *board = zn_scene_ensure_dangling_board(self);

  zn_board_set_screen(board, screen);
  screen->board = board;
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

  wl_list_init(&self->screen_list);
  wl_list_init(&self->board_list);

  return self;

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
}

void
zn_scene_destroy(struct zn_scene *self)
{
  wl_list_remove(&self->screen_list);
  free(self);
}