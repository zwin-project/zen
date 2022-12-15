#include "shell.h"

#include <cglm/quat.h>
#include <float.h>
#include <zen-common.h>
#include <zgnr/bounded.h>

#include "board.h"
#include "bounded.h"
#include "expansive.h"
#include "seat-capsule.h"
#include "zen/board.h"
#include "zen/cursor.h"
#include "zen/scene.h"
#include "zen/server.h"
#include "zen/virtual-object.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
bool
zn_shell_root_ray_cast(void *user_data, vec3 origin, vec3 direction,
    mat4 transform, float *distance)
{
  return false;
}

static bool
zn_shell_root_ray_motion(void *user_data, vec3 origin, vec3 direction,
    uint32_t time_msec, mat4 transform)
{
  return true;
}

static bool
zn_shell_root_ray_enter(void *user_data, uint32_t serial, vec3 origin,
    vec3 direction, mat4 transform)
{
  return true;
}

static bool
zn_shell_root_ray_leave(void *user_data, uint32_t serial, mat4 transform)
{
  return true;
}

static bool
zn_shell_root_ray_button(void *user_data, uint32_t serial, uint32_t time_msec,
    uint32_t button, enum zgn_ray_button_state state, mat4 transform)
{
  return true;
}
#pragma GCC diagnostic pop

static const struct zns_node_interface node_implementation = {
    .ray_cast = zn_shell_root_ray_cast,
    .ray_motion = zn_shell_root_ray_motion,
    .ray_enter = zn_shell_root_ray_enter,
    .ray_leave = zn_shell_root_ray_leave,
    .ray_button = zn_shell_root_ray_button,
};

void
zn_shell_handle_new_display_system(struct zn_shell *self)
{
  UNUSED(self);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_ray *ray = server->scene->ray;
  struct zn_cursor *cursor = server->scene->cursor;

  if (server->display_system == ZN_DISPLAY_SYSTEM_SCREEN) {
    struct zn_board *board = cursor->grab->cursor->board;
    double x = cursor->x;
    double y = cursor->y;
    ray->grab->impl->cancel(ray->grab);
    cursor->grab->impl->enter(cursor->grab, board, x, y);
  } else {
    cursor->grab->impl->leave(cursor->grab);
    ray->grab->impl->rebase(ray->grab);
  }
}

static void
zn_shell_handle_new_bounded(struct wl_listener *listener, void *data)
{
  struct zn_shell *self = zn_container_of(listener, self, new_bounded_listener);

  struct zgnr_bounded *zgnr_bounded = data;

  (void)zns_bounded_create(zgnr_bounded);
}

static void
zn_shell_handle_new_expansive(struct wl_listener *listener, void *data)
{
  struct zn_shell *self =
      zn_container_of(listener, self, new_expansive_listener);

  struct zgnr_expansive *zgnr_expansive = data;

  (void)zns_expansive_create(zgnr_expansive);
}

static void
zn_shell_handle_new_board(struct wl_listener *listener, void *data)
{
  struct zn_shell *self = zn_container_of(listener, self, new_board_listener);

  struct zn_board *zn_board = data;

  struct zns_board *zns_board = zns_board_create(zn_board);

  zns_seat_capsule_add_board(self->seat_capsule, zns_board);
}

struct zn_ray_grab *
zn_shell_get_default_grab(struct zn_shell *self)
{
  return &self->default_grab.base;
}

struct zn_shell *
zn_shell_create(struct wl_display *display, struct zn_scene *scene)
{
  struct zn_shell *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_shell = zgnr_shell_create(display);
  if (self->zgnr_shell == NULL) {
    zn_error("Failed to create zgnr_shell");
    goto err_free;
  }

  self->seat_capsule = zns_seat_capsule_create();
  if (self->seat_capsule == NULL) {
    zn_error("Failed to create a seat capsule");
    goto err_zgnr_shell;
  }

  self->root = zns_node_create(NULL, self, &node_implementation);
  if (self->root == NULL) {
    zn_error("Failed to create zns_node");
    goto err_seat_capsule;
  }

  zns_default_ray_grab_init(&self->default_grab, self);

  self->new_bounded_listener.notify = zn_shell_handle_new_bounded;
  wl_signal_add(
      &self->zgnr_shell->events.new_bounded, &self->new_bounded_listener);

  self->new_expansive_listener.notify = zn_shell_handle_new_expansive;
  wl_signal_add(
      &self->zgnr_shell->events.new_expansive, &self->new_expansive_listener);

  self->new_board_listener.notify = zn_shell_handle_new_board;
  wl_signal_add(&scene->events.new_board, &self->new_board_listener);

  return self;

err_seat_capsule:
  zns_seat_capsule_destroy(self->seat_capsule);

err_zgnr_shell:
  zgnr_shell_destroy(self->zgnr_shell);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_shell_destroy(struct zn_shell *self)
{
  zns_default_ray_grab_fini(&self->default_grab);
  wl_list_remove(&self->new_board_listener.link);
  wl_list_remove(&self->new_expansive_listener.link);
  wl_list_remove(&self->new_bounded_listener.link);
  zns_node_destroy(self->root);
  zns_seat_capsule_destroy(self->seat_capsule);
  zgnr_shell_destroy(self->zgnr_shell);
  free(self);
}
