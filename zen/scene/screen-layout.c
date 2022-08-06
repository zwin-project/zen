#include "zen/scene/screen-layout.h"

#include "zen-common.h"
#include "zen/scene/screen.h"

struct zn_screen*
zn_screen_layout_get_screen_at(struct zn_screen_layout* self, int x, int y)
{
  struct zn_screen* screen;

  wl_list_for_each(screen, &self->screens, link)
  {
    if (wlr_box_contains_point(&screen->box, x, y)) {
      return screen;
    }
  }

  return NULL;
}

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
    zn_screen_get_box(screen, &box);
    screen->x = x;
    screen->y = 0;
    x += box.width;
  }

  wl_signal_emit(&self->events.new_screen, new_screen);
}

void
zn_screen_layout_remove(struct zn_screen_layout* self, struct zn_screen* screen)
{
  UNUSED(self);
  wl_list_remove(&screen->link);
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
  wl_signal_init(&self->events.new_screen);

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
