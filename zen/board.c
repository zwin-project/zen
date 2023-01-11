#include "zen/board.h"

#include <cglm/quat.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <time.h>
#include <zen-common.h>

#include "zen/appearance/board.h"
#include "zen/screen-layout.h"
#include "zen/screen.h"
#include "zen/server.h"
#include "zen/view.h"

#define BOARD_PIXEL_PER_METER 2800.f

void
zn_board_rearrange_view(struct zn_board *self)
{
  struct zn_view *view;
  uint32_t i = 1;
  wl_list_for_each (view, &self->view_list, board_link) {
    zn_view_update_z_index(view, i++);
  }
}

bool
zn_board_is_dangling(struct zn_board *self)
{
  return self->screen == NULL;
}

void
zn_board_send_frame_done(struct zn_board *self, struct timespec *when)
{
  struct zn_view *view;

  wl_list_for_each (view, &self->view_list, board_link) {
    wlr_surface_send_frame_done(view->surface, when);

    // TODO: Popups, subsurfaces?
  }

  // TODO: client-defined cursor
}

void
zn_board_move(struct zn_board *self, vec2 size, mat4 transform)
{
  glm_vec2_copy(size, self->geometry.size);
  glm_mat4_copy(transform, self->geometry.transform);
}

struct wlr_surface *
zn_board_get_surface_at(struct zn_board *self, double x, double y,
    double *surface_x, double *surface_y, struct zn_view **view)
{
  struct zn_view *view_iterator;
  double view_sx, view_sy;
  struct wlr_surface *surface;

  wl_list_for_each_reverse (view_iterator, &self->view_list, board_link) {
    struct wlr_fbox fbox;
    zn_view_get_surface_fbox(view_iterator, &fbox);
    view_sx = x - fbox.x;
    view_sy = y - fbox.y;

    double sx, sy;
    surface = view_iterator->impl->get_wlr_surface_at(
        view_iterator, view_sx, view_sy, &sx, &sy);
    if (surface) {
      if (surface_x) *surface_x = sx;
      if (surface_y) *surface_y = sy;
      if (view) *view = view_iterator;
      return surface;
    }
  }

  return NULL;
}

void
zn_board_get_effective_size(
    struct zn_board *self, double *width, double *height)
{
  struct zn_server *server = zn_server_get_singleton();
  if (server->display_system == ZN_DISPLAY_SYSTEM_SCREEN) {
    if (self->screen) {
      zn_screen_get_effective_size(self->screen, width, height);
    } else {
      *width = 0;
      *height = 0;
    }
  } else {
    *width = self->geometry.size[0] * BOARD_PIXEL_PER_METER;
    *height = self->geometry.size[1] * BOARD_PIXEL_PER_METER;
  }
}

void
zn_board_box_effective_to_local_geom(
    struct zn_board *self, struct wlr_fbox *effective, struct wlr_fbox *geom)
{
  UNUSED(self);
  geom->x = effective->x / BOARD_PIXEL_PER_METER - self->geometry.size[0] / 2.f;
  geom->y = self->geometry.size[1] - effective->y / BOARD_PIXEL_PER_METER;
  geom->width = effective->width / BOARD_PIXEL_PER_METER;
  geom->height = effective->height / BOARD_PIXEL_PER_METER;
}

static void
zn_board_handle_screen_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_board *self =
      zn_container_of(listener, self, screen_destroy_listener);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_screen_layout *screen_layout = server->scene->screen_layout;

  struct zn_screen *screen = NULL;
  if (!wl_list_empty(&screen_layout->screen_list)) {
    screen = zn_container_of(screen_layout->screen_list.next, screen, link);
  }

  zn_board_set_screen(self, screen);
}

void
zn_board_set_screen(struct zn_board *self, struct zn_screen *screen)
{
  if (self->screen) {
    wl_list_remove(&self->screen_destroy_listener.link);
    wl_list_init(&self->screen_destroy_listener.link);
    wl_list_remove(&self->screen_link);
    wl_list_init(&self->screen_link);
  }

  if (screen) {
    wl_signal_add(&screen->events.destroy, &self->screen_destroy_listener);
    wl_list_insert(screen->board_list.prev, &self->screen_link);
  }

  self->screen = screen;

  if (self->screen) {
    double width, height;
    zn_screen_get_effective_size(screen, &width, &height);
    glm_vec2_copy(
        (vec2){width / BOARD_PIXEL_PER_METER, height / BOARD_PIXEL_PER_METER},
        self->geometry.size);
  }
}

struct zn_board *
zn_board_create(void)
{
  struct zn_board *self;
  struct timespec time;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->appearance = zna_board_create(self, server->appearance_system);
  if (self->appearance == NULL) {
    goto err_free;
  }

  wl_list_init(&self->link);
  wl_list_init(&self->screen_link);
  self->screen = NULL;
  wl_list_init(&self->view_list);

  self->screen_destroy_listener.notify = zn_board_handle_screen_destroy;
  wl_list_init(&self->screen_destroy_listener.link);

  wl_signal_init(&self->events.destroy);

  clock_gettime(CLOCK_MONOTONIC, &time);

  glm_mat4_identity(self->geometry.transform);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_board_destroy(struct zn_board *self)
{
  wl_list_remove(&self->screen_link);
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->view_list);
  wl_list_remove(&self->screen_destroy_listener.link);
  wl_list_remove(&self->link);
  zna_board_destroy(self->appearance);
  free(self);
}
