#include "zen/scene/board.h"

#include "zen-common.h"

static void
zn_board_handle_screen_destroy(struct wl_listener *listener, void *data)
{
  struct zn_board *self =
      zn_container_of(listener, self, screen_destroy_listener);
  UNUSED(data);

  zn_board_assign_to_screen(self, NULL);
}

static void
zn_board_resize(struct zn_board *self, double width, double height)
{
  self->width = width;
  self->height = height;
  // TODO: move views inside the resized board
}

bool
zn_board_is_dangling(struct zn_board *self)
{
  return self->screen == NULL;
}

void
zn_board_assign_to_screen(struct zn_board *self, struct zn_screen *screen)
{
  struct zn_board_screen_assigned_event event;
  if (self->screen == screen) {
    return;
  }

  if (!zn_board_is_dangling(self)) {
    wl_list_remove(&self->screen_link);
    wl_list_init(&self->screen_link);

    wl_list_remove(&self->screen_destroy_listener.link);
    wl_list_init(&self->screen_destroy_listener.link);
  }

  if (screen) {
    struct wlr_fbox box;

    wl_list_insert(screen->board_list.prev, &self->screen_link);
    wl_signal_add(&screen->events.destroy, &self->screen_destroy_listener);

    zn_screen_get_fbox(screen, &box);
    zn_board_resize(self, box.width, box.height);
  }

  event.prev_screen = self->screen;
  event.current_screen = screen;

  self->screen = screen;

  wl_signal_emit(&self->events.screen_assigned, &event);
}

struct zn_board *
zn_board_create(void)
{
  struct zn_board *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->width = 1920;
  self->height = 1080;

  wl_list_init(&self->view_list);

  wl_list_init(&self->screen_link);
  self->screen = NULL;

  self->screen_destroy_listener.notify = zn_board_handle_screen_destroy;
  wl_list_init(&self->screen_destroy_listener.link);

  wl_signal_init(&self->events.screen_assigned);
  wl_signal_init(&self->events.destroy);

  return self;

err:
  return NULL;
}

void
zn_board_destroy(struct zn_board *self)
{
  wl_signal_emit(&self->events.destroy, NULL);
  zn_board_assign_to_screen(self, NULL);
  wl_list_remove(&self->view_list);
  wl_list_remove(&self->link);
  free(self);
}
