#include "zen/scene/screen-layout.h"

#include "zen-common.h"
#include "zen/scene/screen.h"

void
zn_screen_layout_add(
    struct zn_screen_layout* self, struct zn_screen* new_screen)
{
  int x = 0;
  struct zn_screen* screen;
  struct wlr_box box;
  wl_list_insert(&self->screens, &new_screen->link);
  wl_list_for_each(screen, &self->screens, link)
  {
    wlr_output_effective_resolution(
        screen->output->wlr_output, &box.width, &box.height);
    screen->x = x;
    screen->y = 0;
    x += box.width;
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
