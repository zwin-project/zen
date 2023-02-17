#include "zen-desktop/shell.h"

#include <wayland-server-core.h>
#include <wayland-util.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/screen-container.h"
#include "zen-desktop/screen-layout.h"
#include "zen/backend.h"
#include "zen/server.h"

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

  self->new_screen_listener.notify = zn_desktop_shell_handle_new_screen;
  wl_signal_add(
      &server->backend->events.new_screen, &self->new_screen_listener);

  return self;

err_free:
  desktop_shell_singleton = NULL;
  free(self);

err:
  return NULL;
}

void
zn_desktop_shell_destroy(struct zn_desktop_shell *self)
{
  wl_list_remove(&self->new_screen_listener.link);
  zn_screen_layout_destroy(self->screen_layout);
  desktop_shell_singleton = NULL;
  free(self);
}
