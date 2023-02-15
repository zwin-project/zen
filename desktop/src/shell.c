#include "zen-desktop/shell.h"

#include <wayland-server-core.h>
#include <wayland-util.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend.h"
#include "zen/server.h"

static void
zn_desktop_shell_handle_new_screen(
    struct wl_listener *listener UNUSED, void *data UNUSED)
{
  // TODO(@Aki-7): Implement here
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

  self->new_screen_listener.notify = zn_desktop_shell_handle_new_screen;
  wl_signal_add(
      &server->backend->events.new_screen, &self->new_screen_listener);

  return self;

err:
  return NULL;
}

void
zn_desktop_shell_destroy(struct zn_desktop_shell *self)
{
  wl_list_remove(&self->new_screen_listener.link);
  free(self);
}
