#include "server.h"

#include <wayland-server.h>

#include "zen-common/log.h"
#include "zen-common/util.h"

struct zn_server {
  struct wl_display *display;

  int exit_code;
};
int
zn_server_run(struct zn_server *self)
{
  wl_display_run(self->display);
  return self->exit_code;
}

void
zn_server_terminate(struct zn_server *self, int exit_code)
{
  self->exit_code = exit_code;
  wl_display_terminate(self->display);
}

struct zn_server *
zn_server_create(struct wl_display *display)
{
  struct zn_server *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;
  self->exit_code = EXIT_FAILURE;

  return self;

err:
  return NULL;
}

void
zn_server_destroy(struct zn_server *self)
{
  free(self);
}
