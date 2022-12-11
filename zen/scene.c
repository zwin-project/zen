#include "zen/scene.h"

#include <zen-common.h>

#include "zen/board.h"
#include "zen/cursor.h"
#include "zen/ray.h"
#include "zen/screen.h"
#include "zen/server.h"
#include "zen/space.h"
#include "zen/view.h"
#include "zen/wlr/texture.h"

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

static void
zn_scene_set_current_space(struct zn_scene *self, struct zn_space *space)
{
  if (self->current_space) {
    wl_list_remove(&self->current_space_destroy_listener.link);
    wl_list_init(&self->current_space_destroy_listener.link);
    zgnr_space_leave(self->current_space->zgnr_space);
    zgnr_space_shutdown(self->current_space->zgnr_space);
  }

  if (space) {
    wl_signal_add(
        &space->events.destroy, &self->current_space_destroy_listener);
    zgnr_space_enter(space->zgnr_space);
  }

  self->current_space = space;
}

void
zn_scene_new_space(struct zn_scene *self, struct zn_space *space)
{
  zn_scene_set_current_space(self, space);
}

static void
zn_scene_handle_current_space_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_scene *self =
      zn_container_of(listener, self, current_space_destroy_listener);

  zn_scene_set_current_space(self, NULL);
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

static void
zn_scene_setup_wallpaper(struct zn_scene *self, const char *wallpaper_filepath)
{
  cairo_surface_t *surface =
      cairo_image_surface_create_from_png(wallpaper_filepath);
  cairo_t *cr = cairo_create(surface);
  cairo_status_t status = cairo_status(cr);
  if (status != CAIRO_STATUS_SUCCESS) {
    zn_warn("Wallpaper not loaded");
    goto err;
  }
  cairo_format_t format = cairo_image_surface_get_format(surface);
  if (format != CAIRO_FORMAT_ARGB32 && format != CAIRO_FORMAT_RGB24) {
    zn_error("Image format not supported");
    goto err;
  }

  struct zn_server *server = zn_server_get_singleton();
  self->wallpaper =
      zn_wlr_texture_from_cairo_surface(surface, server->renderer);
err:
  cairo_surface_destroy(surface);
  cairo_destroy(cr);
}

void
zn_scene_new_view(struct zn_scene *self, struct zn_view *view)
{
  wl_list_insert(&self->view_list, &view->link);

  if (wl_list_empty(&self->board_list)) return;

  struct zn_board *board = zn_container_of(self->board_list.next, board, link);

  zn_view_move(view, board, 0, 0);

  zna_view_commit(view->appearance, ZNA_VIEW_DAMAGE_GEOMETRY);
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

  struct zn_server *server = zn_server_get_singleton();
  zn_scene_setup_wallpaper(self, server->config->wallpaper_filepath);

  wl_list_init(&self->screen_list);
  wl_list_init(&self->board_list);
  wl_list_init(&self->view_list);
  wl_signal_init(&self->events.new_board);

  self->current_space_destroy_listener.notify =
      zn_scene_handle_current_space_destroy;
  wl_list_init(&self->current_space_destroy_listener.link);

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
  wl_list_remove(&self->current_space_destroy_listener.link);
  wl_list_remove(&self->screen_list);
  wl_list_remove(&self->view_list);
  if (self->wallpaper != NULL) wlr_texture_destroy(self->wallpaper);
  zn_cursor_destroy(self->cursor);
  zn_ray_destroy(self->ray);
  free(self);
}
