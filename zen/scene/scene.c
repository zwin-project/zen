#include "zen/scene/scene.h"

#include <cairo.h>
#include <drm_fourcc.h>
#include <linux/input.h>

#include "build-config.h"
#include "zen-common.h"
#include "zen/config.h"
#include "zen/scene/board.h"
#include "zen/scene/screen-layout.h"
#include "zen/scene/screen.h"
#include "zen/scene/view.h"

static void
zn_scene_handle_unmap_focused_view(struct wl_listener* listener, void* data)
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

static struct zn_board*
zn_scene_new_board(struct zn_scene* self)
{
  struct zn_board* board;

  board = zn_board_create();
  if (board == NULL) {
    zn_error("Failed to create a new board");
    return NULL;
  }

  wl_list_insert(self->board_list.prev, &board->link);

  wl_signal_emit(&self->events.new_board, board);

  return board;
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

  board = zn_scene_new_board(self);
  if (board == NULL) {
    return;
  }

  zn_board_assign_to_screen(board, screen);
  zn_screen_set_current_board(screen, board);
}

struct zn_ray*
zn_scene_ensure_ray(struct zn_scene* self)
{
  if (self->ray == NULL) {
    self->ray = zn_ray_create();
    wl_signal_emit(&self->events.new_ray, NULL);
  }

  return self->ray;
}

void
zn_scene_destroy_ray(struct zn_scene* self)
{
  if (self->ray) zn_ray_destroy(self->ray);
  self->ray = NULL;
}

void
zn_scene_set_focused_view(struct zn_scene* self, struct zn_view* view)
{
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_seat* seat = server->input_manager->seat->wlr_seat;

  if (view == self->focused_view) {
    return;
  }

  if (self->focused_view != NULL) {
    self->focused_view->impl->set_activated(self->focused_view, false);
    if (self->focused_view->impl->close_popups)
      self->focused_view->impl->close_popups(self->focused_view);
    wl_list_remove(&self->unmap_focused_view_listener.link);
    wl_list_init(&self->unmap_focused_view_listener.link);

    wlr_seat_keyboard_notify_clear_focus(seat);
  }

  if (view != NULL) {
    view->impl->set_activated(view, true);
    zn_view_bring_to_front(view);
    wl_signal_add(&view->events.unmap, &self->unmap_focused_view_listener);

    struct wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);
    wlr_seat_keyboard_notify_enter(seat, view->impl->get_wlr_surface(view),
        keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
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
  wl_list_for_each (board, &self->board_list, link) {
    if (zn_board_is_dangling(board)) {
      return board;
    }
  }

  return zn_scene_new_board(self);
}

/** Keep this idempotent. */
void
zn_scene_reassign_boards(struct zn_scene* self)
{
  struct zn_screen* screen;
  struct zn_board* board;

  // assign a board to a screen without board
  wl_list_for_each (screen, &self->screen_layout->screens, link) {
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
    wl_list_for_each (board, &self->board_list, link) {
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

static void
zn_scene_setup_background(struct zn_scene* self, const char* background_png)
{
  cairo_surface_t* surface =
      cairo_image_surface_create_from_png(background_png);
  cairo_t* cr = cairo_create(surface);
  cairo_status_t status = cairo_status(cr);
  if (status != CAIRO_STATUS_SUCCESS) {
    zn_warn("Background image not loaded");
    goto err;
  }
  cairo_format_t format = cairo_image_surface_get_format(surface);
  if (format != CAIRO_FORMAT_ARGB32 && format != CAIRO_FORMAT_RGB24) {
    zn_error("Image format not supported");
    goto err;
  }
  unsigned char* data = cairo_image_surface_get_data(surface);
  int stride = cairo_image_surface_get_stride(surface);
  int width = cairo_image_surface_get_width(surface);
  int height = cairo_image_surface_get_height(surface);

  struct zn_server* server = zn_server_get_singleton();
  self->bg_texture = wlr_texture_from_pixels(
      server->renderer, DRM_FORMAT_ARGB8888, stride, width, height, data);
err:
  cairo_surface_destroy(surface);
  cairo_destroy(cr);
}

struct zn_scene*
zn_scene_create(struct zn_config* config)
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

  zn_scene_setup_background(self, config->bg_image_file);

  self->unmap_focused_view_listener.notify = zn_scene_handle_unmap_focused_view;
  wl_list_init(&self->unmap_focused_view_listener.link);

  wl_signal_init(&self->events.new_board);
  wl_signal_init(&self->events.new_ray);

  board = zn_scene_new_board(self);
  if (board == NULL) {
    goto err_bg_texture;
  }

  return self;

err_bg_texture:
  if (self->bg_texture != NULL) {
    wlr_texture_destroy(self->bg_texture);
  }

  zn_screen_layout_destroy(self->screen_layout);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_scene_destroy_resources(struct zn_scene* self)
{
  if (self->bg_texture != NULL) {
    wlr_texture_destroy(self->bg_texture);
  }
}

void
zn_scene_destroy(struct zn_scene* self)
{
  struct zn_board *board, *tmp;

  wl_list_for_each_safe (board, tmp, &self->board_list, link) {
    zn_board_destroy(board);
  }

  zn_screen_layout_destroy(self->screen_layout);

  wl_list_remove(&self->events.new_board.listener_list);
  wl_list_remove(&self->events.new_ray.listener_list);
  wl_list_remove(&self->unmap_focused_view_listener.link);

  free(self);
}
