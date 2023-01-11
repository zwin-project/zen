#include "zen/scene.h"

#include <linux/input.h>
#include <zen-common.h>

#include "zen/board.h"
#include "zen/cursor.h"
#include "zen/ray.h"
#include "zen/screen-layout.h"
#include "zen/screen.h"
#include "zen/server.h"
#include "zen/view.h"
#include "zen/wlr/texture.h"

static void
zn_scene_handle_switch_board_binding(
    uint32_t time_msec, uint32_t key, void *data)
{
  UNUSED(time_msec);
  struct zn_scene *scene = data;
  struct zn_screen *screen = scene->cursor->board->screen;

  if (screen == NULL) return;

  if (key == KEY_RIGHT) {
    zn_screen_switch_to_next_board(screen);
  } else if (key == KEY_LEFT) {
    zn_screen_switch_to_prev_board(screen);
  }
}

static void
zn_scene_handle_focused_view_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_scene *self =
      zn_container_of(listener, self, focused_view_destroy_listener);

  zn_scene_set_focused_view(self, NULL);
}

static void
zn_scene_handle_display_system_changed(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_scene *self =
      zn_container_of(listener, self, display_system_changed_listener);
  enum zn_display_system_state *state = data;

  if (*state == ZN_DISPLAY_SYSTEM_SCREEN &&
      !zn_cursor_is_visible_in_screen(self->cursor) &&
      !wl_list_empty(&self->screen_layout->screen_list)) {
    struct zn_screen *screen =
        zn_container_of(self->screen_layout->screen_list.next, screen, link);
    if (screen->current_board) {
      double width, height;
      zn_screen_get_effective_size(screen, &width, &height);
      zn_cursor_move(
          self->cursor, screen->current_board, width / 2, height / 2);
      self->cursor->grab->impl->rebase(self->cursor->grab);
    }
  }
}

static struct zn_board *
zn_scene_create_new_board(struct zn_scene *self)
{
  struct zn_board *board = zn_board_create();

  wl_list_insert(&self->board_list, &board->link);

  wl_signal_emit(&self->events.new_board, board);

  return board;
}

void
zn_scene_new_screen(struct zn_scene *self, struct zn_screen *screen)
{
  zn_screen_layout_add(self->screen_layout, screen);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_board *board = NULL;

  struct zn_board *board_iter = NULL;
  wl_list_for_each (board_iter, &self->board_list, link) {
    if (zn_board_is_dangling(board_iter)) {
      if (!board) {
        board = board_iter;
      }
      zn_board_set_screen(board_iter, screen);
      wl_signal_emit(&self->events.board_mapped_to_screen, board_iter);
    }
  }

  if (wl_list_empty(&screen->board_list)) {
    board = zn_scene_create_new_board(self);
    zn_board_set_screen(board, screen);
    wl_signal_emit(&self->events.board_mapped_to_screen, board);
  }

  zn_screen_set_current_board(screen, board);

  if (server->display_system == ZN_DISPLAY_SYSTEM_SCREEN &&
      zn_screen_layout_screen_count(self->screen_layout) == 1) {
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

  struct zn_board *board = self->cursor->board;

  if (!board) {
    if (wl_list_empty(&self->board_list)) return;
    board = zn_container_of(self->board_list.next, board, link);
  }

  struct wlr_fbox view_fbox;
  double board_width, board_height;
  zn_view_get_view_fbox(view, &view_fbox);
  zn_board_get_effective_size(board, &board_width, &board_height);
  zn_view_move(view, board, (board_width - view_fbox.width) / 2,
      (board_height - view_fbox.height) / 2);

  zn_scene_set_focused_view(self, view);

  zn_view_commit_appearance(view);
}

void
zn_scene_set_focused_view(struct zn_scene *self, struct zn_view *view)
{
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_seat *seat = server->input_manager->seat->wlr_seat;

  if (view == self->focused_view) {
    return;
  }

  if (self->focused_view != NULL) {
    self->focused_view->impl->set_activated(self->focused_view, false);
    wl_list_remove(&self->focused_view_destroy_listener.link);
    wl_list_init(&self->focused_view_destroy_listener.link);

    wlr_seat_keyboard_notify_clear_focus(seat);
  }

  if (view != NULL) {
    view->impl->set_activated(view, true);
    wl_signal_add(&view->events.destroy, &self->focused_view_destroy_listener);
    zn_view_bring_to_front(view);

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    wlr_seat_keyboard_notify_enter(seat, view->surface, keyboard->keycodes,
        keyboard->num_keycodes, &keyboard->modifiers);
  }

  self->focused_view = view;
}

void
zn_scene_initialize_boards(struct zn_scene *self, int64_t board_initial_count)
{
  struct zn_server *server = zn_server_get_singleton();
  for (int i = 0; i < board_initial_count; ++i) {
    zn_scene_create_new_board(self);
  }
  zn_shell_rearrange_board(server->shell);
}

void
zn_scene_setup_keybindings(struct zn_scene *self)
{
  struct zn_server *server = zn_server_get_singleton();
  zn_input_manager_add_key_binding(server->input_manager, KEY_RIGHT,
      WLR_MODIFIER_SHIFT | WLR_MODIFIER_LOGO,
      zn_scene_handle_switch_board_binding, self);

  zn_input_manager_add_key_binding(server->input_manager, KEY_LEFT,
      WLR_MODIFIER_SHIFT | WLR_MODIFIER_LOGO,
      zn_scene_handle_switch_board_binding, self);
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

  self->screen_layout = zn_screen_layout_create();
  if (self->screen_layout == NULL) {
    zn_error("Failed to create a screen_layout");
    goto err_free;
  }

  self->ray = zn_ray_create();
  if (self->ray == NULL) {
    zn_error("Failed to create a ray");
    goto err_screen_layout;
  }

  self->cursor = zn_cursor_create();
  if (self->cursor == NULL) {
    zn_error("Failed to creat a cursor");
    goto err_ray;
  }

  struct zn_server *server = zn_server_get_singleton();
  zn_scene_setup_wallpaper(self, server->config->wallpaper_filepath);

  self->focused_view_destroy_listener.notify =
      zn_scene_handle_focused_view_destroy;
  wl_list_init(&self->focused_view_destroy_listener.link);

  self->display_system_changed_listener.notify =
      zn_scene_handle_display_system_changed;
  wl_signal_add(&server->events.display_system_changed,
      &self->display_system_changed_listener);

  wl_list_init(&self->board_list);
  wl_list_init(&self->view_list);
  wl_signal_init(&self->events.new_board);
  wl_signal_init(&self->events.board_mapped_to_screen);

  return self;

err_ray:
  zn_ray_destroy(self->ray);

err_screen_layout:
  zn_screen_layout_destroy(self->screen_layout);

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
  wl_list_remove(&self->focused_view_destroy_listener.link);
  wl_list_remove(&self->events.new_board.listener_list);
  wl_list_remove(&self->events.board_mapped_to_screen.listener_list);
  wl_list_remove(&self->display_system_changed_listener.link);
  wl_list_remove(&self->view_list);
  if (self->wallpaper != NULL) wlr_texture_destroy(self->wallpaper);
  zn_screen_layout_destroy(self->screen_layout);
  zn_cursor_destroy(self->cursor);
  zn_ray_destroy(self->ray);
  free(self);
}
