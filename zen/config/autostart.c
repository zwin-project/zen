#include "zen/config/autostart.h"

#include <string.h>
#include <zen-common.h>

void
zn_autostart_exec(struct zn_autostart *self)
{
  launch_command(self->command);
}

struct zn_autostart *
zn_autostart_create(char *command)
{
  struct zn_autostart *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->link);

  self->command = strdup(command);

  return self;

err:
  return NULL;
}

void
zn_autostart_destroy(struct zn_autostart *self)
{
  wl_list_remove(&self->link);
  free(self->command);
  free(self);
}
