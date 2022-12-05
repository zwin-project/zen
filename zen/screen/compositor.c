#include "zen/screen/compositor.h"

#include <zen-common.h>

#include "zen/screen/xdg-toplevel.h"

static void
zn_screen_compositor_handle_xdg_shell_new_surface(
    struct wl_listener *listener, void *data)
{
  struct zn_screen_compositor *self =
      zn_container_of(listener, self, xdg_shell_new_surface_listener);
  struct wlr_xdg_surface *xdg_surface = data;

  UNUSED(self);
  UNUSED(xdg_surface);
  if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
    zn_xdg_toplevel_create(xdg_surface->toplevel);
    return;
  }

  // TODO: handle other roles
}

struct zn_screen_compositor *
zn_screen_compositor_create(
    struct wl_display *display, struct wlr_renderer *renderer)
{
  struct zn_screen_compositor *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;

  self->compositor = wlr_compositor_create(display, renderer);
  if (self->compositor == NULL) {
    zn_error("Failed to create a wlr_compositor");
    goto err_free;
  }

  self->xdg_shell = wlr_xdg_shell_create(display);
  if (self->xdg_shell == NULL) {
    zn_error("Failed to create a wlr_xdg_shell");
    goto err_free;
  }

  self->xdg_shell_new_surface_listener.notify =
      zn_screen_compositor_handle_xdg_shell_new_surface;
  wl_signal_add(&self->xdg_shell->events.new_surface,
      &self->xdg_shell_new_surface_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_screen_compositor_destroy(struct zn_screen_compositor *self)
{
  wl_list_remove(&self->xdg_shell_new_surface_listener.link);
  free(self);
}
