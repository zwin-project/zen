#include "zen/view.h"

#include <cglm/quat.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <zen-common.h>

#include "zen/board.h"
#include "zen/screen.h"
#include "zen/server.h"

static void
zn_view_handle_board_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_view *self =
      zn_container_of(listener, self, board_destroy_listener);
  zn_view_move(self, NULL, 0, 0);

  zna_view_commit(self->appearance, ZNA_VIEW_DAMAGE_GEOMETRY);
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
zn_view_damage_whole(struct zn_view *self)
{
  struct wlr_fbox fbox;

  zn_view_get_surface_fbox(self, &fbox);

  zn_view_add_damage_fbox(self, &fbox);
}

void
zn_view_get_surface_fbox(struct zn_view *self, struct wlr_fbox *fbox)
{
  // FIXME: take window geometry into account
  fbox->x = self->x;
  fbox->y = self->y;
  fbox->width = self->surface->current.width;
  fbox->height = self->surface->current.height;
}

void
zn_view_move(struct zn_view *self, struct zn_board *board, double x, double y)
{
  if (self->board != board) {
    if (self->board) {
      wl_list_remove(&self->board_destroy_listener.link);
      wl_list_init(&self->board_destroy_listener.link);
      wl_list_remove(&self->board_link);
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

  if (self->board) {
    struct wlr_fbox view_fbox, board_local_view_geom;
    mat4 transform, board_rotation;
    glm_quat_mat4(self->board->geometry.quaternion, board_rotation);

    glm_vec4_copy(self->board->geometry.quaternion, self->geometry.quaternion);

    // FIXME: take window geometry into account
    zn_view_get_surface_fbox(self, &view_fbox);
    zn_board_box_effective_to_local_geom(
        self->board, &view_fbox, &board_local_view_geom);

    self->geometry.size[0] = board_local_view_geom.width;
    self->geometry.size[1] = board_local_view_geom.height;

    glm_vec3_zero(self->geometry.position);
    glm_mat4_identity(transform);
    glm_translate(transform, self->board->geometry.center);
    glm_mat4_mul(transform, board_rotation, transform);
    // FIXME: take view overlap (z-index) into account
    glm_translate(
        transform, (vec3){board_local_view_geom.x, board_local_view_geom.y,
                       VIEW_Z_OFFSET_ON_BOARD});
    glm_mat4_mulv3(
        transform, self->geometry.position, 1, self->geometry.position);

  } else {
    glm_vec3_zero(self->geometry.position);
    glm_vec2_zero(self->geometry.size);
    glm_quat_identity(self->geometry.quaternion);
  }
}

struct zn_view *
zn_view_create(struct wlr_surface *surface)
{
  struct zn_view *self;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->surface = surface;

  self->appearance = zna_view_create(self, server->appearance_system);
  if (self->appearance == NULL) {
    zn_error("Failed to create a zna_view");
    goto err_free;
  }

  wl_list_init(&self->link);
  wl_list_init(&self->board_link);

  glm_vec3_zero(self->geometry.position);
  glm_vec2_zero(self->geometry.size);
  glm_quat_identity(self->geometry.quaternion);

  wl_signal_init(&self->events.destroy);

  self->board_destroy_listener.notify = zn_view_handle_board_destroy;
  wl_list_init(&self->board_destroy_listener.link);

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

  wl_list_remove(&self->link);
  wl_list_remove(&self->board_link);
  wl_list_remove(&self->board_destroy_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  zna_view_destroy(self->appearance);
  free(self);
}