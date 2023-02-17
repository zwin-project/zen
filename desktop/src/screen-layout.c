#include "zen-desktop/screen-layout.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/screen-container.h"
#include "zen/screen.h"

void
zn_screen_layout_reposition(struct zn_screen_layout *self)
{
  // TODO(@Aki-7): it's a naive implementation
  float x = 0;
  struct zn_screen_container *container = NULL;
  wl_list_for_each (container, &self->screen_container_list, link) {
    container->position[0] = x;
    container->position[1] = 0;
    x += container->screen->size[0];
  }
}

void
zn_screen_layout_add(
    struct zn_screen_layout *self, struct zn_screen_container *container)
{
  wl_list_insert(self->screen_container_list.prev, &container->link);

  zn_screen_layout_reposition(self);
}

struct zn_screen_layout *
zn_screen_layout_create(void)
{
  struct zn_screen_layout *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->screen_container_list);

  return self;

err:
  return NULL;
}

void
zn_screen_layout_destroy(struct zn_screen_layout *self)
{
  wl_list_remove(&self->screen_container_list);
  free(self);
}
