#include "zen/scene/view.h"

#include <stdbool.h>

#include "zen-common.h"
#include "zen/scene/board.h"
#include "zen/input/seat.h"
#include "zen/xdg-toplevel-view.h"
#include "zen/xwayland-view.h"

void
zn_view_get_fbox(struct zn_view *self, struct wlr_fbox *fbox)
{
  struct wlr_surface *surface = self->impl->get_wlr_surface(self);

  fbox->x = self->x;
  fbox->y = self->y;
  fbox->width = surface->current.width;
  fbox->height = surface->current.height;
}

bool
zn_view_is_mapped(struct zn_view *self)
{
  // check if some zn_board has this view in zn_board::view_list
  return !wl_list_empty(&self->link);
}

void
zn_view_map_to_scene(struct zn_view *self, struct zn_scene *scene)
{
  struct zn_board *board;
  struct wlr_fbox fbox;

  board = zn_scene_get_focus_board(scene);

  if (board == NULL && zn_assert(!wl_list_empty(&scene->board_list),
                           "zn_scene::board_list should not be empty")) {
    board = zn_container_of(scene->board_list.next, board, link);
  }

  if (board == NULL) {
    zn_error("Failed to find a board to which view is mapped");
    return;
  }

  // TODO: handle board destruction

  zn_view_get_fbox(self, &fbox);
  self->x = (board->width - fbox.width) / 2;
  self->y = (board->height - fbox.height) / 2;

  wl_list_insert(&board->view_list, &self->link);
}

void
zn_view_unmap(struct zn_view *self)
{
  wl_list_remove(&self->link);
  wl_list_init(&self->link);
}

void
zn_view_init(struct zn_view *self, enum zn_view_type type,
    const struct zn_view_impl *impl)
{
  self->type = type;
  self->impl = impl;
  wl_list_init(&self->link);
}

void
zn_view_fini(struct zn_view *self)
{
  wl_list_remove(&self->link);
}
