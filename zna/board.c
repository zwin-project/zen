#include "board.h"

#include <GLES3/gl32.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>
#include <zen-common.h>

#include "system.h"

void
zna_board_commit(struct zna_board *self)
{
  zna_board_plane_unit_commit(
      self->plane_unit, self->zn_board, self->virtual_object);
}

/**
 * @param session must not be null
 */
static void
zna_board_setup_renderer_object(
    struct zna_board *self, struct znr_session *session)
{
  UNUSED(session);
  struct znr_dispatcher *dispatcher = self->system->dispatcher;
  self->virtual_object = znr_virtual_object_create(dispatcher);
  zna_board_plane_unit_setup_renderer_objects(
      self->plane_unit, dispatcher, self->virtual_object);

  zna_board_commit(self);
}

static void
zna_board_teardown_renderer_object(struct zna_board *self)
{
  zna_board_plane_unit_teardown_renderer_objects(self->plane_unit);

  if (self->virtual_object) {
    znr_virtual_object_destroy(self->virtual_object);
    self->virtual_object = NULL;
  }
}

static void
zna_board_handle_session_frame(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_board *self =
      zn_container_of(listener, self, session_frame_listener);

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  zn_board_send_frame_done(self->zn_board, &now);
}

static void
zna_board_handle_session_created(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_board *self =
      zn_container_of(listener, self, session_created_listener);
  struct znr_session *session = self->system->current_session;

  zna_board_setup_renderer_object(self, session);
}

static void
zna_board_handle_session_destroyed(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zna_board *self =
      zn_container_of(listener, self, session_destroyed_listener);

  zna_board_teardown_renderer_object(self);
}

struct zna_board *
zna_board_create(struct zn_board *board, struct zna_system *system)
{
  struct zna_board *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zn_board = board;
  self->system = system;
  self->virtual_object = NULL;

  self->plane_unit = zna_board_plane_unit_create(system);
  if (self->plane_unit == NULL) {
    zn_error("Failed to create a zna_board_plane_unit");
    goto err_free;
  }

  self->session_created_listener.notify = zna_board_handle_session_created;
  wl_signal_add(
      &system->events.current_session_created, &self->session_created_listener);

  self->session_destroyed_listener.notify = zna_board_handle_session_destroyed;
  wl_signal_add(&system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  self->session_frame_listener.notify = zna_board_handle_session_frame;
  wl_signal_add(
      &system->events.current_session_frame, &self->session_frame_listener);

  struct znr_session *session = self->system->current_session;
  if (session) {
    zna_board_setup_renderer_object(self, session);
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zna_board_destroy(struct zna_board *self)
{
  if (self->virtual_object) znr_virtual_object_destroy(self->virtual_object);
  zna_board_plane_unit_destroy(self->plane_unit);
  wl_list_remove(&self->session_created_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  wl_list_remove(&self->session_frame_listener.link);
  free(self);
}
