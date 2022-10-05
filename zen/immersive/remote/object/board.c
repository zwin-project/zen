#include "zen/immersive/remote/object/board.h"

#include "zen-common.h"

static void
zn_board_remote_object_handle_board_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_board_remote_object* self =
      zn_container_of(listener, self, board_destroy_listener);

  zn_board_remote_object_destroy(self);
}

struct zn_board_remote_object*
zn_board_remote_object_create(
    struct zn_board* board, struct zn_remote_scene* remote_scene)
{
  struct zn_board_remote_object* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->board = board;
  self->board_destroy_listener.notify =
      zn_board_remote_object_handle_board_destroy;
  wl_signal_add(&board->events.destroy, &self->board_destroy_listener);

  wl_list_insert(&remote_scene->board_object_list, &self->link);

  return self;

err:
  return NULL;
}

void
zn_board_remote_object_destroy(struct zn_board_remote_object* self)
{
  wl_list_remove(&self->link);
  wl_list_remove(&self->board_destroy_listener.link);
  free(self);
}
