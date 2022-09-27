#include "zen/scene/view.h"

#include <stdbool.h>
#include <wlr/types/wlr_output_damage.h>

#include "zen-common.h"
#include "zen/input/seat.h"
#include "zen/scene/board.h"
#include "zen/xdg-toplevel-view.h"
#include "zen/xwayland-view.h"

void
zn_view_bring_to_front(struct zn_view *self)
{
  wl_list_remove(&self->link);
  wl_list_insert(self->board->view_list.prev, &self->link);

  if (self->impl->restack) {
    self->impl->restack(self, XCB_STACK_MODE_ABOVE);
  }

  zn_view_damage_whole(self);
}

void
zn_view_move(struct zn_view *self, struct zn_board *new_board, double board_x,
    double board_y)
{
  zn_view_damage_whole(self);

  if (new_board != self->board) {
    if (self->board) {
      wl_list_remove(&self->link);
      wl_list_init(&self->link);
    }
    if (new_board) {
      wl_list_insert(new_board->view_list.prev, &self->link);
    }
    self->board = new_board;
  }

  self->x = board_x;
  self->y = board_y;

  if (self->impl->set_position) {
    self->impl->set_position(self, board_x, board_y);
  }

  zn_view_damage_whole(self);
}

static void
zn_view_add_damage_fbox(struct zn_view *self, struct wlr_fbox *effective_box)
{
  struct zn_board *board = self->board;
  struct zn_screen *screen = board ? board->screen : NULL;

  if (screen == NULL) {
    return;
  }

  zn_output_add_damage_box(screen->output, effective_box);
}

void
zn_view_damage(struct zn_view *self)
{
  struct wlr_surface *surface = self->impl->get_wlr_surface(self);
  struct wlr_fbox damage_box;
  pixman_region32_t damage;
  pixman_box32_t *rects;
  int rect_count;

  pixman_region32_init(&damage);

  wlr_surface_get_effective_damage(surface, &damage);
  rects = pixman_region32_rectangles(&damage, &rect_count);

  for (int i = 0; i < rect_count; ++i) {
    damage_box = (struct wlr_fbox){
        .x = self->x + rects[i].x1,
        .y = self->y + rects[i].y1,
        .width = rects[i].x2 - rects[i].x1,
        .height = rects[i].y2 - rects[i].y1,
    };
    zn_view_add_damage_fbox(self, &damage_box);
  }

  pixman_region32_fini(&damage);

  // FIXME: add damages of synced subsurfaces
}

void
zn_view_damage_whole(struct zn_view *self)
{
  struct wlr_fbox fbox;

  if (zn_view_has_client_decoration(self)) {
    zn_view_get_surface_fbox(self, &fbox);
  } else {
    zn_view_get_decoration_fbox(self, &fbox);
  }

  zn_view_add_damage_fbox(self, &fbox);
}

void
zn_view_get_surface_fbox(struct zn_view *self, struct wlr_fbox *fbox)
{
  struct wlr_surface *surface = self->impl->get_wlr_surface(self);

  fbox->x = self->x;
  fbox->y = self->y;
  fbox->width = surface->current.width;
  fbox->height = surface->current.height;
}

void
zn_view_get_window_fbox(struct zn_view *self, struct wlr_fbox *fbox)
{
  struct wlr_box view_geometry;
  self->impl->get_geometry(self, &view_geometry);

  fbox->x = view_geometry.x + self->x;
  fbox->y = view_geometry.y + self->y;
  fbox->width = view_geometry.width;
  fbox->height = view_geometry.height;
}

void
zn_view_get_decoration_fbox(struct zn_view *self, struct wlr_fbox *fbox)
{
  zn_view_get_surface_fbox(self, fbox);
  fbox->x -= VIEW_DECORATION_BORDER;
  fbox->y -= VIEW_DECORATION_BORDER + VIEW_DECORATION_TITLEBAR;
  fbox->width += VIEW_DECORATION_BORDER * 2;
  fbox->height += VIEW_DECORATION_BORDER * 2 + VIEW_DECORATION_TITLEBAR;
}

bool
zn_view_has_client_decoration(struct zn_view *self)
{
  struct wlr_box box;
  self->impl->get_geometry(self, &box);
  return (box.x != 0 || box.y != 0) || self->requested_client_decoration;
}

bool
zn_view_is_mapped(struct zn_view *self)
{
  return self->board != NULL;
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

  zn_view_get_window_fbox(self, &fbox);
  zn_view_move(self, board, (board->width - fbox.width) / 2,
      (board->height - fbox.height) / 2);

  zn_scene_set_focused_view(scene, self);

  zn_view_damage_whole(self);
}

void
zn_view_unmap(struct zn_view *self)
{
  wl_signal_emit(&self->events.unmap, NULL);

  zn_view_damage_whole(self);

  zn_view_move(self, NULL, 0, 0);
}

void
zn_view_init(struct zn_view *self, enum zn_view_type type,
    const struct zn_view_impl *impl)
{
  self->type = type;
  self->impl = impl;
  self->board = NULL;

  wl_signal_init(&self->events.unmap);
  wl_signal_init(&self->events.destroy);
  wl_list_init(&self->link);
}

void
zn_view_fini(struct zn_view *self)
{
  wl_signal_emit(&self->events.destroy, NULL);
  wl_list_remove(&self->link);
}
