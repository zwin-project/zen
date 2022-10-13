#include "zen/immersive/remote/scene.h"

#include "zen-common.h"
#include "zen/immersive/remote/object/board.h"

static void
zn_remote_scene_handle_new_board(struct wl_listener* listener, void* data)
{
  struct zn_remote_scene* self =
      zn_container_of(listener, self, new_board_listener);

  struct zn_board* board = data;

  zn_board_remote_object_create(board, self);
}

static void
zn_remote_scene_clean(struct zn_remote_scene* self)
{
  struct zn_board_remote_object *board_object, *tmp;

  wl_list_for_each_safe (board_object, tmp, &self->board_object_list, link) {
    zn_board_remote_object_destroy(board_object);
  }
}

static void
zn_remote_scene_clean_build(struct zn_remote_scene* self)
{
  struct zn_board* board;

  zn_remote_scene_clean(self);

  wl_list_for_each (board, &self->scene->board_list, link) {
    zn_board_remote_object_create(board, self);
  }
}

void
zn_remote_scene_start_sync(struct zn_remote_scene* self)
{
  zn_remote_scene_clean_build(self);

  wl_signal_add(&self->scene->signals.new_board, &self->new_board_listener);
}

void
zn_remote_scene_stop_sync(struct zn_remote_scene* self)
{
  wl_list_remove(&self->new_board_listener.link);
  wl_list_init(&self->new_board_listener.link);

  zn_remote_scene_clean(self);
}

struct zn_remote_scene*
zn_remote_scene_create(struct zn_scene* scene, struct znr_remote* remote)
{
  struct zn_remote_scene* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->scene = scene;
  self->remote = remote;

  wl_list_init(&self->board_object_list);

  self->new_board_listener.notify = zn_remote_scene_handle_new_board;
  wl_list_init(&self->new_board_listener.link);

  return self;

err:
  return NULL;
}

void
zn_remote_scene_destroy(struct zn_remote_scene* self)
{
  wl_list_remove(&self->board_object_list);
  wl_list_remove(&self->new_board_listener.link);
  free(self);
}
