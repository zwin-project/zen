#include "zen/scene/screen-layout.h"

#include "zen-common.h"
#include "zen/scene/screen.h"

void
zn_screen_layout_add(
    struct zn_screen_layout* self, struct zn_screen* new_screen)
{
  wl_list_insert(&self->screens, &new_screen->link);
  int x = 0;
  struct zn_screen* screen;
  wl_list_for_each(screen, &self->screens, link)
  {
    screen->x = x;
    screen->y = 0;
    x += screen->output->wlr_output->width;
    zn_debug("added: %s (%4d, %d) ", screen->output->wlr_output->name,
        screen->x, screen->y);
  }
}

struct zn_screen_layout*
zn_screen_layout_create(void)
{
  struct zn_screen_layout* self;
  self = zalloc(sizeof *self);

  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->screens);

  return self;

err:
  return NULL;
}

void
zn_screen_layout_destroy(struct zn_screen_layout* self)
{
  wl_list_remove(&self->screens);
  free(self);
}
