#include "shell.h"

#include <zen-common.h>

struct zn_shell*
zn_shell_create(struct wl_display* display)
{
  struct zn_shell* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_shell = zgnr_shell_create(display);
  if (self->zgnr_shell == NULL) {
    zn_error("Failed to create zgnr_shell");
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_shell_destroy(struct zn_shell* self)
{
  zgnr_shell_destroy(self->zgnr_shell);
  free(self);
}
