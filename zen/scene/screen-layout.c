#include "zen/scene/screen-layout.h"

#include <math.h>

#include "zen-common.h"
#include "zen/scene/screen.h"

struct zn_screen*
zn_screen_layout_get_closest_screen(
    struct zn_screen_layout* self, int x, int y, int* dst_x, int* dst_y)
{
  double current_closest_x, current_closest_y, current_closest_distance;
  double closest_x, closest_y, closest_distance;
  struct wlr_box box;
  struct zn_screen* closest_screen = NULL;
  struct zn_screen* screen;

  wl_list_for_each(screen, &self->screens, link)
  {
    zn_screen_get_box(screen, &box);
    wlr_box_closest_point(&box, x, y, &current_closest_x, &current_closest_y);
    current_closest_distance =
        pow(x - current_closest_x, 2) + pow(y - current_closest_y, 2);
    if (closest_screen == NULL || current_closest_distance < closest_distance) {
      closest_x = current_closest_x;
      closest_y = current_closest_y;
      closest_distance = current_closest_distance;
      closest_screen = screen;
    }
  }

  *dst_x = closest_x - closest_screen->x;
  *dst_y = closest_y - closest_screen->y;
  return closest_screen;
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
