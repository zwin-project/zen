#include "zen/view.h"

#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <wlr/util/edges.h>
#include <zen-common.h>

#include "zen/board.h"
#include "zen/screen.h"
#include "zen/server.h"

static void
zn_view_update_geometry(struct zn_view *self)
{
  if (self->board) {
    struct wlr_fbox view_fbox, board_local_view_geom;

    // FIXME: take window geometry into account
    zn_view_get_surface_fbox(self, &view_fbox);
    zn_board_box_effective_to_local_geom(
        self->board, &view_fbox, &board_local_view_geom);

    self->geometry.size[0] = board_local_view_geom.width;
    self->geometry.size[1] = board_local_view_geom.height;

    glm_mat4_copy(self->board->geometry.transform, self->geometry.transform);
    glm_translate(self->geometry.transform,
        (vec3){board_local_view_geom.x, board_local_view_geom.y,
            VIEW_MIN_Z_OFFSET_ON_BOARD + VIEW_Z_OFFSET_GAP * self->z_index});
  } else {
    glm_mat4_identity(self->geometry.transform);
    glm_vec2_zero(self->geometry.size);
  }

  self->appearance_damage |= ZNA_VIEW_DAMAGE_GEOMETRY;
}

void
zn_view_update_z_index(struct zn_view *self, uint32_t z_index)
{
  if (self->z_index == z_index) {
    return;
  }
  self->z_index = z_index;
  zn_view_update_geometry(self);
}

static void
zn_view_handle_board_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_view *self =
      zn_container_of(listener, self, board_destroy_listener);
  zn_view_move(self, NULL, 0, 0);

  zn_view_commit_appearance(self);
}

static void
zn_view_add_damage_fbox(struct zn_view *self, struct wlr_fbox *effective_box)
{
  struct zn_board *board = self->board;
  struct zn_screen *screen = board ? board->screen : NULL;

  if (screen != NULL) {
    zn_screen_damage(screen, effective_box);
  }
}

static void
zn_view_handle_commit(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_view *self = zn_container_of(listener, self, commit_listener);
  struct wlr_fbox damage_box, surface_box;
  pixman_region32_t damage;
  pixman_box32_t *rects;
  int rect_count;

  zn_view_get_surface_fbox(self, &surface_box);

  pixman_region32_init(&damage);

  wlr_surface_get_effective_damage(self->surface, &damage);
  rects = pixman_region32_rectangles(&damage, &rect_count);

  struct wlr_box window_geom;
  self->impl->get_window_geom(self, &window_geom);

  // geometry was changed
  if (self->prev_surface_fbox.x != -window_geom.x ||
      self->prev_surface_fbox.y != -window_geom.y) {
    damage_box = self->prev_surface_fbox;
    damage_box.x += self->x;
    damage_box.y += self->y;
    zn_view_add_damage_fbox(self, &damage_box);

    // add damage with *updated* surface box
    zn_view_get_surface_fbox(self, &damage_box);
    zn_view_add_damage_fbox(self, &damage_box);
  } else {
    for (int i = 0; i < rect_count; ++i) {
      damage_box.x = surface_box.x + rects[i].x1;
      damage_box.y = surface_box.y + rects[i].y1;
      damage_box.width = rects[i].x2 - rects[i].x1;
      damage_box.height = rects[i].y2 - rects[i].y1;
      zn_view_add_damage_fbox(self, &damage_box);
    }
  }

  if (pixman_region32_not_empty(&damage)) {
    // FIXME: Update only sub image
    self->appearance_damage |= ZNA_VIEW_DAMAGE_TEXTURE;
  }

  pixman_region32_fini(&damage);

  // FIXME: add damages of synced subsurfaces

  const uint32_t last_serial = self->impl->get_current_configure_serial(self);

  if (self->maximize_status.changed_serial == last_serial) {
    self->maximize_status.changed_serial = 0;
    if (self->maximize_status.maximized) {
      zn_view_move(self, self->board, 0, 0);
    } else {
      zn_view_move(self, self->board, self->maximize_status.reset_box.x,
          self->maximize_status.reset_box.y);
    }
  }

  if (self->resize_status.resizing &&
      self->resize_status.edges & (WLR_EDGE_LEFT | WLR_EDGE_TOP)) {
    struct wlr_surface *surface = data;
    int dx = 0, dy = 0;
    if (self->resize_status.edges & WLR_EDGE_LEFT) {
      dx = surface->previous.width - surface->current.width;
    }
    if (self->resize_status.edges & WLR_EDGE_TOP) {
      dy = surface->previous.height - surface->current.height;
    }
    zn_view_move(self, self->board, self->x + dx, self->y + dy);

    if (self->resize_status.last_serial == last_serial) {
      self->resize_status.resizing = false;
    }
  }

  self->prev_surface_fbox.x = -window_geom.x;
  self->prev_surface_fbox.y = -window_geom.y;
  self->prev_surface_fbox.width = surface_box.width;
  self->prev_surface_fbox.height = surface_box.height;

  // FIXME: Update geometry more efficiently
  zn_view_update_geometry(self);

  zn_view_commit_appearance(self);
}

static void
zn_view_damage_whole(struct zn_view *self)
{
  struct wlr_fbox fbox;

  zn_view_get_surface_fbox(self, &fbox);

  zn_view_add_damage_fbox(self, &fbox);
}

void
zn_view_commit_appearance(struct zn_view *self)
{
  if (self->appearance_damage != 0) {
    zna_view_commit(self->_appearance, self->appearance_damage);
    self->appearance_damage = 0;
  }
}

void
zn_view_get_surface_fbox(struct zn_view *self, struct wlr_fbox *fbox)
{
  struct wlr_box window_geom;
  self->impl->get_window_geom(self, &window_geom);

  fbox->x = self->x - window_geom.x;
  fbox->y = self->y - window_geom.y;
  fbox->width = self->surface->current.width;
  fbox->height = self->surface->current.height;
}

void
zn_view_get_view_fbox(struct zn_view *self, struct wlr_fbox *fbox)
{
  struct wlr_box window_geom;
  self->impl->get_window_geom(self, &window_geom);

  fbox->x = self->x;
  fbox->y = self->y;
  fbox->width = window_geom.width;
  fbox->height = window_geom.height;
}

void
zn_view_bring_to_front(struct zn_view *self)
{
  if (!self->board) {
    return;
  }

  wl_list_remove(&self->board_link);
  wl_list_insert(self->board->view_list.prev, &self->board_link);
  zn_board_rearrange_view(self->board);

  zn_view_damage_whole(self);
}

void
zn_view_move(struct zn_view *self, struct zn_board *board, double x, double y)
{
  if (self->board != board) {
    if (self->board) {
      wl_list_remove(&self->board_destroy_listener.link);
      wl_list_init(&self->board_destroy_listener.link);
      wl_list_remove(&self->board_link);
      wl_list_init(&self->board_link);
    }

    if (board) {
      wl_signal_add(&board->events.destroy, &self->board_destroy_listener);
      wl_list_insert(&board->view_list, &self->board_link);
    }
  }

  zn_view_damage_whole(self);

  self->x = x;
  self->y = y;

  self->board = board;

  zn_view_damage_whole(self);

  zn_view_update_geometry(self);
}

void
zn_view_set_maximized(struct zn_view *self, bool maximized)
{
  // TODO: handle in a better way
  if (self == NULL) return;
  if (!self->board || self->maximize_status.maximized == maximized) {
    self->impl->schedule_configure(self);
    return;
  }

  double width, height;
  if (maximized) {
    zn_view_get_view_fbox(self, &self->maximize_status.reset_box);
    zn_board_get_effective_size(self->board, &width, &height);
  } else {
    width = self->maximize_status.reset_box.width;
    height = self->maximize_status.reset_box.height;
  }

  self->impl->set_maximized(self, maximized);
  self->maximize_status.changed_serial =
      self->impl->set_size(self, width, height);
  self->maximize_status.maximized = maximized;
}

struct zn_view *
zn_view_create(struct wlr_surface *surface,
    const struct zn_view_interface *impl, void *user_data)
{
  struct zn_view *self;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->surface = surface;
  self->impl = impl;
  self->user_data = user_data;

  self->_appearance = zna_view_create(self, server->appearance_system);
  if (self->_appearance == NULL) {
    zn_error("Failed to create a zna_view");
    goto err_free;
  }

  wl_list_init(&self->link);
  wl_list_init(&self->board_link);

  glm_vec2_zero(self->geometry.size);
  glm_mat4_identity(self->geometry.transform);

  wl_signal_init(&self->events.destroy);

  self->board_destroy_listener.notify = zn_view_handle_board_destroy;
  wl_list_init(&self->board_destroy_listener.link);

  self->commit_listener.notify = zn_view_handle_commit;
  wl_signal_add(&surface->events.commit, &self->commit_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_view_destroy(struct zn_view *self)
{
  zn_view_damage_whole(self);
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->board_link);
  zn_board_rearrange_view(self->board);

  wl_list_remove(&self->link);
  wl_list_remove(&self->board_destroy_listener.link);
  wl_list_remove(&self->commit_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  zna_view_destroy(self->_appearance);
  free(self);
}
