#include "zen/view-child.h"

#include <cglm/affine.h>
#include <cglm/vec2.h>
#include <zen-common.h>

#include "zen/appearance/view-child.h"
#include "zen/board.h"
#include "zen/screen.h"
#include "zen/server.h"
#include "zen/view.h"

static void
zn_view_child_commit_appearance(struct zn_view_child *self)
{
  if (self->appearance_damage != 0) {
    zna_view_child_commit(self->_appearance, self->appearance_damage);
    self->appearance_damage = 0;
  }
}

static void
zn_view_child_get_surface_fbox(
    struct zn_view_child *self, struct wlr_fbox *fbox)
{
  struct wlr_fbox surface_fbox;
  zn_view_get_surface_fbox(self->view, &surface_fbox);

  double sx, sy;
  self->impl->get_toplevel_coords(self, 0, 0, &sx, &sy);

  fbox->x = surface_fbox.x + sx;
  fbox->y = surface_fbox.y + sy;
  fbox->width = self->surface->current.width;
  fbox->height = self->surface->current.height;
}

static void
zn_view_child_update_geometry(struct zn_view_child *self)
{
  if (self->view->board) {
    struct wlr_fbox view_fbox, board_local_view_geom;

    zn_view_child_get_surface_fbox(self, &view_fbox);
    zn_board_box_effective_to_local_geom(
        self->view->board, &view_fbox, &board_local_view_geom);

    self->geometry.size[0] = board_local_view_geom.width;
    self->geometry.size[1] = board_local_view_geom.height;

    glm_mat4_copy(
        self->view->board->geometry.transform, self->geometry.transform);
    // TODO: better z-index calculating
    glm_translate(self->geometry.transform,
        (vec3){board_local_view_geom.x, board_local_view_geom.y,
            VIEW_MIN_Z_OFFSET_ON_BOARD +
                VIEW_Z_OFFSET_GAP * (self->view->z_index + 1)});
  } else {
    glm_mat4_identity(self->geometry.transform);
    glm_vec2_zero(self->geometry.size);
  }

  self->appearance_damage |= ZNA_VIEW_DAMAGE_GEOMETRY;
}

static void
zn_view_child_add_damage_fbox(
    struct zn_view_child *self, struct wlr_fbox *effective_box)
{
  struct zn_board *board = self->view->board;
  struct zn_screen *screen = board ? board->screen : NULL;

  if (screen != NULL) {
    zn_screen_damage(screen, effective_box);
  }
}

static void
zn_view_child_damage_whole(struct zn_view_child *self)
{
  struct wlr_fbox fbox;

  zn_view_child_get_surface_fbox(self, &fbox);

  zn_view_child_add_damage_fbox(self, &fbox);
}

static void
zn_view_child_handle_commit(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_view_child *self = zn_container_of(listener, self, commit_listener);
  struct wlr_fbox damage_box, surface_box;
  pixman_region32_t damage;
  pixman_box32_t *rects;
  int rect_count;

  zn_view_child_get_surface_fbox(self, &surface_box);

  pixman_region32_init(&damage);

  wlr_surface_get_effective_damage(self->surface, &damage);
  rects = pixman_region32_rectangles(&damage, &rect_count);

  for (int i = 0; i < rect_count; ++i) {
    damage_box.x = surface_box.x + rects[i].x1;
    damage_box.y = surface_box.y + rects[i].y1;
    damage_box.width = rects[i].x2 - rects[i].x1;
    damage_box.height = rects[i].y2 - rects[i].y1;
    zn_view_child_add_damage_fbox(self, &damage_box);
  }

  if (pixman_region32_not_empty(&damage)) {
    self->appearance_damage |= ZNA_VIEW_DAMAGE_TEXTURE;
  }

  pixman_region32_fini(&damage);

  // FIXME: Update geometry more efficiently
  zn_view_child_update_geometry(self);

  zn_view_child_commit_appearance(self);
}

struct zn_view_child *
zn_view_child_create(struct wlr_surface *surface, struct zn_view *view,
    const struct zn_view_child_interface *impl, void *user_data)
{
  struct zn_view_child *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->user_data = user_data;
  self->surface = surface;
  self->surface_on_partial_updates = false;
  self->impl = impl;
  self->view = view;

  struct zn_server *server = zn_server_get_singleton();
  self->_appearance = zna_view_child_create(self, server->appearance_system);
  if (self->_appearance == NULL) {
    zn_error("Failed to create a zna_view_child");
    goto err_free;
  }

  glm_vec2_zero(self->geometry.size);
  glm_mat4_identity(self->geometry.transform);

  self->commit_listener.notify = zn_view_child_handle_commit;
  wl_signal_add(&surface->events.commit, &self->commit_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_view_child_destroy(struct zn_view_child *self)
{
  zn_view_child_damage_whole(self);
  wl_list_remove(&self->commit_listener.link);
  zna_view_child_destroy(self->_appearance);
  free(self);
}
