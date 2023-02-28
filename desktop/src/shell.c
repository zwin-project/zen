#include "zen-desktop/shell.h"

#include <wayland-server-core.h>
#include <wayland-util.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/cursor-grab/default.h"
#include "zen-desktop/screen-container.h"
#include "zen-desktop/screen-layout.h"
#include "zen/backend.h"
#include "zen/screen.h"
#include "zen/seat.h"
#include "zen/server.h"
#include "zen/snode.h"
#include "zen/view.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static struct zn_desktop_shell *desktop_shell_singleton = NULL;

static void
zn_desktop_shell_handle_new_screen(struct wl_listener *listener, void *data)
{
  struct zn_desktop_shell *self =
      zn_container_of(listener, self, new_screen_listener);
  struct zn_screen *screen = data;
  struct zn_screen_container *container = zn_screen_container_create(screen);

  zn_screen_layout_add(self->screen_layout, container);
}

static void
zn_desktop_shell_handle_view_mapped(struct wl_listener *listener, void *data)
{
  struct zn_desktop_shell *self =
      zn_container_of(listener, self, view_mapped_listener);
  struct zn_view *view = data;
  struct zn_screen *screen =
      zn_screen_layout_get_main_screen(self->screen_layout);

  if (screen) {
    zn_snode_set_position(view->snode, screen->snode_root, (vec2){0, 0});
  }
}

static void
zn_desktop_shell_handle_pointer_motion(struct wl_listener *listener, void *data)
{
  struct zn_desktop_shell *self =
      zn_container_of(listener, self, pointer_motion_listener);
  struct wlr_event_pointer_motion *event = data;
  vec2 delta = {(float)event->delta_x, (float)event->delta_y};

  zn_cursor_grab_pointer_motion(self->cursor_grab, delta, event->time_msec);
}

struct zn_desktop_shell *
zn_desktop_shell_get_singleton(void)
{
  zn_assert(desktop_shell_singleton, "zn_desktop_shell is not initialized yet");
  return desktop_shell_singleton;
}

struct zn_desktop_shell *
zn_desktop_shell_create(void)
{
  struct zn_server *server = zn_server_get_singleton();

  struct zn_desktop_shell *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  desktop_shell_singleton = self;

  self->screen_layout = zn_screen_layout_create();
  if (self->screen_layout == NULL) {
    zn_error("Failed to create a zn_screen_layout");
    goto err_free;
  }

  struct zn_cursor_default_grab *cursor_default_grab =
      zn_cursor_default_grab_create();
  if (cursor_default_grab == NULL) {
    zn_error("Failed to create a cursor default grab");
    goto err_screen_layout;
  }
  self->cursor_grab = &cursor_default_grab->base;

  self->new_screen_listener.notify = zn_desktop_shell_handle_new_screen;
  wl_signal_add(
      &server->backend->events.new_screen, &self->new_screen_listener);

  self->pointer_motion_listener.notify = zn_desktop_shell_handle_pointer_motion;
  wl_signal_add(
      &server->seat->events.pointer_motion, &self->pointer_motion_listener);

  self->view_mapped_listener.notify = zn_desktop_shell_handle_view_mapped;
  wl_signal_add(
      &server->backend->events.view_mapped, &self->view_mapped_listener);

  return self;

err_screen_layout:
  zn_screen_layout_destroy(self->screen_layout);

err_free:
  desktop_shell_singleton = NULL;
  free(self);

err:
  return NULL;
}

void
zn_desktop_shell_destroy(struct zn_desktop_shell *self)
{
  wl_list_remove(&self->view_mapped_listener.link);
  wl_list_remove(&self->pointer_motion_listener.link);
  wl_list_remove(&self->new_screen_listener.link);
  zn_cursor_grab_destroy(self->cursor_grab);
  zn_screen_layout_destroy(self->screen_layout);
  desktop_shell_singleton = NULL;
  free(self);
}