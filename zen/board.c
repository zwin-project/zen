#include "zen/board.h"

#include <time.h>
#include <zen-common.h>

#include "zen/screen.h"

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

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->link);
  self->screen = NULL;

  self->screen_destroy_listener.notify = zn_board_handle_screen_destroy;
  wl_list_init(&self->screen_destroy_listener.link);

  clock_gettime(CLOCK_MONOTONIC, &time);
  self->color[0] = (float)(time.tv_nsec % 255) / 255.f;
  self->color[1] = (float)(time.tv_nsec % 254) / 254.f;
  self->color[2] = (float)(time.tv_nsec % 253) / 253.f;

  return self;

err:
  return NULL;
}

void
zn_board_destroy(struct zn_board *self)
{
  wl_list_remove(&self->screen_destroy_listener.link);
  wl_list_remove(&self->link);
  free(self);
}
