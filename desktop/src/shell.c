#include "zen-desktop/shell.h"

#include <cglm/vec2.h>
#include <wayland-server-core.h>
#include <wayland-util.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/cursor-grab/default.h"
#include "zen-desktop/screen-layout.h"
#include "zen-desktop/screen.h"
#include "zen-desktop/theme.h"
#include "zen-desktop/view.h"
#include "zen-desktop/xr-system.h"
#include "zen/backend.h"
#include "zen/bounded.h"
#include "zen/cursor.h"
#include "zen/inode.h"
#include "zen/screen.h"
#include "zen/seat.h"
#include "zen/server.h"
#include "zen/snode.h"
#include "zen/view.h"
#include "zen/virtual-object.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static struct zn_desktop_shell *desktop_shell_singleton = NULL;

static void
zn_desktop_shell_handle_xr_system_changed(
    struct wl_listener *listener, void *data)
{
  struct zn_desktop_shell *self =
      zn_container_of(listener, self, xr_system_changed_listener);
  struct zn_xr_system *xr_system = data;
  struct zn_server *server = zn_server_get_singleton();

  enum zn_desktop_shell_display_mode mode =
      xr_system ? ZN_DESKTOP_SHELL_DISPLAY_MODE_IMMERSIVE
                : ZN_DESKTOP_SHELL_DISPLAY_MODE_SCREEN;

  if (self->mode == mode) {
    return;
  }

  self->mode = mode;

  if (self->mode == ZN_DESKTOP_SHELL_DISPLAY_MODE_SCREEN) {
    zn_inode_unmap(server->inode_root);
    zn_info("Switch to screen display mode");
  } else {
    zn_inode_map(server->inode_root);
    zn_info("Switch to immersive display mode");
  }
}

static void
zn_desktop_shell_handle_new_xr_system(
    struct wl_listener *listener UNUSED, void *data)
{
  struct zn_server *server = zn_server_get_singleton();

  struct zn_xr_system *zn_xr_system = data;

  zn_desktop_xr_system_create(zn_xr_system, server->display);
}

static void
zn_desktop_shell_handle_new_screen(struct wl_listener *listener, void *data)
{
  struct zn_desktop_shell *self =
      zn_container_of(listener, self, new_screen_listener);
  struct zn_screen *screen = data;
  struct zn_desktop_screen *desktop_screen = zn_desktop_screen_create(screen);

  zn_screen_layout_add(self->screen_layout, desktop_screen);

  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->seat->cursor;

  // FIXME(@Aki-7): when it's immersive display system, the cursor should not
  // appear on the screen
  if (cursor->snode->root == NULL) {
    vec2 position;

    glm_vec2_divs(desktop_screen->screen->size, 2.0F, position);

    zn_snode_set_position(
        cursor->snode, desktop_screen->cursor_layer, position);
  }
}

static void
zn_desktop_shell_handle_view_mapped(struct wl_listener *listener, void *data)
{
  struct zn_desktop_shell *self =
      zn_container_of(listener, self, view_mapped_listener);
  struct zn_view *zn_view = data;

  struct zn_desktop_view *view = zn_desktop_view_create(zn_view);

  struct zn_desktop_screen *desktop_screen =
      zn_screen_layout_get_main_screen(self->screen_layout);

  vec2 initial_position;
  glm_vec2_sub(
      desktop_screen->screen->size, view->zn_view->size, initial_position);
  glm_vec2_scale(initial_position, 0.5F, initial_position);
  if (initial_position[1] < 40) {
    initial_position[1] = 40;
  }

  if (desktop_screen) {
    zn_snode_set_position(
        view->snode, desktop_screen->view_layer, initial_position);
  }

  struct zn_server *server = zn_server_get_singleton();

  zn_seat_set_focus(server->seat, view->snode);
}

static void
zn_desktop_shell_handle_bounded_mapped(
    struct wl_listener *listener UNUSED, void *data)
{
  struct zn_bounded *bounded = data;

  zn_virtual_object_change_visibility(bounded->virtual_object, true);
}

static void
zn_desktop_shell_handle_seat_capabilities(
    struct wl_listener *listener UNUSED, void *data)
{
  struct zn_server *server = zn_server_get_singleton();
  uint32_t *capabilities = data;

  zn_seat_set_capabilities(server->seat, *capabilities);
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

static void
zn_desktop_shell_handle_pointer_button(struct wl_listener *listener, void *data)
{
  struct zn_desktop_shell *self =
      zn_container_of(listener, self, pointer_button_listener);
  struct wlr_event_pointer_button *event = data;

  zn_cursor_grab_pointer_button(
      self->cursor_grab, event->time_msec, event->button, event->state);
}

static void
zn_desktop_shell_handle_pointer_axis(struct wl_listener *listener, void *data)
{
  struct zn_desktop_shell *self =
      zn_container_of(listener, self, pointer_axis_listener);
  struct wlr_event_pointer_axis *event = data;

  zn_cursor_grab_pointer_axis(self->cursor_grab, event->time_msec,
      event->source, event->orientation, event->delta, event->delta_discrete);
}

static void
zn_desktop_shell_handle_pointer_frame(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_desktop_shell *self =
      zn_container_of(listener, self, pointer_frame_listener);

  zn_cursor_grab_pointer_frame(self->cursor_grab);
}

void
zn_desktop_shell_start_cursor_grab(
    struct zn_desktop_shell *self, struct zn_cursor_grab *grab)
{
  zn_cursor_grab_destroy(self->cursor_grab);

  self->cursor_grab = grab;
}

void
zn_desktop_shell_end_cursor_grab(struct zn_desktop_shell *self)
{
  struct zn_cursor_default_grab *cursor_default_grab =
      zn_cursor_default_grab_create();
  if (cursor_default_grab == NULL) {
    zn_abort("Failed to create a default cursor grab");
    return;
  }

  zn_cursor_grab_destroy(self->cursor_grab);

  self->cursor_grab = &cursor_default_grab->base;

  zn_cursor_grab_pointer_rebase(self->cursor_grab);
}

struct zn_theme *
zn_desktop_shell_get_theme(void)
{
  return zn_desktop_shell_get_singleton()->theme;
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

  self->mode = ZN_DESKTOP_SHELL_DISPLAY_MODE_SCREEN;

  self->theme = zn_theme_create();
  if (self->theme == NULL) {
    zn_error("Failed to create theme");
    goto err_free;
  }

  self->screen_layout = zn_screen_layout_create();
  if (self->screen_layout == NULL) {
    zn_error("Failed to create a zn_screen_layout");
    goto err_theme;
  }

  struct zn_cursor_default_grab *cursor_default_grab =
      zn_cursor_default_grab_create();
  if (cursor_default_grab == NULL) {
    zn_error("Failed to create a cursor default grab");
    goto err_screen_layout;
  }
  self->cursor_grab = &cursor_default_grab->base;

  self->xr_system_changed_listener.notify =
      zn_desktop_shell_handle_xr_system_changed;
  wl_signal_add(&server->backend->events.xr_system_changed,
      &self->xr_system_changed_listener);

  self->new_xr_system_listener.notify = zn_desktop_shell_handle_new_xr_system;
  wl_signal_add(
      &server->backend->events.new_xr_system, &self->new_xr_system_listener);

  self->new_screen_listener.notify = zn_desktop_shell_handle_new_screen;
  wl_signal_add(
      &server->backend->events.new_screen, &self->new_screen_listener);

  self->seat_capabilities_listener.notify =
      zn_desktop_shell_handle_seat_capabilities;
  wl_signal_add(&server->seat->events.update_capabilities,
      &self->seat_capabilities_listener);

  self->pointer_motion_listener.notify = zn_desktop_shell_handle_pointer_motion;
  wl_signal_add(
      &server->seat->events.pointer_motion, &self->pointer_motion_listener);

  self->pointer_button_listener.notify = zn_desktop_shell_handle_pointer_button;
  wl_signal_add(
      &server->seat->events.pointer_button, &self->pointer_button_listener);

  self->pointer_axis_listener.notify = zn_desktop_shell_handle_pointer_axis;
  wl_signal_add(
      &server->seat->events.pointer_axis, &self->pointer_axis_listener);

  self->pointer_frame_listener.notify = zn_desktop_shell_handle_pointer_frame;
  wl_signal_add(
      &server->seat->events.pointer_frame, &self->pointer_frame_listener);

  self->view_mapped_listener.notify = zn_desktop_shell_handle_view_mapped;
  wl_signal_add(
      &server->backend->events.view_mapped, &self->view_mapped_listener);

  self->bounded_mapped_listener.notify = zn_desktop_shell_handle_bounded_mapped;
  wl_signal_add(
      &server->backend->events.bounded_mapped, &self->bounded_mapped_listener);

  return self;

err_screen_layout:
  zn_screen_layout_destroy(self->screen_layout);

err_theme:
  zn_theme_destroy(self->theme);

err_free:
  desktop_shell_singleton = NULL;
  free(self);

err:
  return NULL;
}

void
zn_desktop_shell_destroy(struct zn_desktop_shell *self)
{
  wl_list_remove(&self->bounded_mapped_listener.link);
  wl_list_remove(&self->view_mapped_listener.link);
  wl_list_remove(&self->pointer_frame_listener.link);
  wl_list_remove(&self->pointer_axis_listener.link);
  wl_list_remove(&self->pointer_button_listener.link);
  wl_list_remove(&self->pointer_motion_listener.link);
  wl_list_remove(&self->seat_capabilities_listener.link);
  wl_list_remove(&self->new_screen_listener.link);
  wl_list_remove(&self->new_xr_system_listener.link);
  wl_list_remove(&self->xr_system_changed_listener.link);
  zn_cursor_grab_destroy(self->cursor_grab);
  zn_screen_layout_destroy(self->screen_layout);
  zn_theme_destroy(self->theme);
  desktop_shell_singleton = NULL;
  free(self);
}
