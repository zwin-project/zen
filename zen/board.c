#include "zen/board.h"

#include <cglm/quat.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <time.h>
#include <zen-common.h>

#include "zen/appearance/board.h"
#include "zen/screen.h"
#include "zen/server.h"

bool
zn_board_is_dangling(struct zn_board *self)
{
  return self->screen == NULL;
}

static void
zn_board_handle_screen_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_board *self =
      zn_container_of(listener, self, screen_destroy_listener);

  zn_board_set_screen(self, NULL);
}

void
zn_board_set_screen(struct zn_board *self, struct zn_screen *screen)
{
  if (self->screen) {
    wl_list_remove(&self->screen_destroy_listener.link);
    wl_list_init(&self->screen_destroy_listener.link);
  }

  if (screen) {
    wl_signal_add(&screen->events.destroy, &self->screen_destroy_listener);
  }

  self->screen = screen;
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
  self->screen = NULL;

  self->screen_destroy_listener.notify = zn_board_handle_screen_destroy;
  wl_list_init(&self->screen_destroy_listener.link);

  clock_gettime(CLOCK_MONOTONIC, &time);
  self->color[0] = (float)(time.tv_nsec % 255) / 255.f;
  self->color[1] = (float)(time.tv_nsec % 254) / 254.f;
  self->color[2] = (float)(time.tv_nsec % 253) / 253.f;

  glm_vec3_copy((vec3){0, 1, -0.5}, self->geometry.center);
  glm_quat_identity(self->geometry.quaternion);
  glm_vec2_copy((vec2){0.7, 0.5}, self->geometry.size);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_board_destroy(struct zn_board *self)
{
  wl_list_remove(&self->screen_destroy_listener.link);
  wl_list_remove(&self->link);
  zna_board_destroy(self->appearance);
  free(self);
}
