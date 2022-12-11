#include "zen/screen-layout.h"

#include <float.h>
#include <zen-common.h>

#include "zen/screen.h"
#include "zen/wlr/box.h"

static void
zn_screen_layout_rearrange(struct zn_screen_layout *self)
{
  int x = 0;
  struct zn_screen *screen;
  double width, height;

  wl_list_for_each (screen, &self->screen_list, link) {
    zn_screen_get_effective_size(screen, &width, &height);
    screen->x = x;
    screen->y = 0;
    x += width;
  }
}

struct zn_screen *
zn_screen_layout_get_closest_screen(struct zn_screen_layout *self, double x,
    double y, double *dst_x, double *dst_y)
{
  double closest_x = 0, closest_y = 0, closest_distance = DBL_MAX;
  struct zn_screen *closest_screen = NULL;
  struct zn_screen *screen;

  wl_list_for_each (screen, &self->screen_list, link) {
    double current_closest_x, current_closest_y, current_closest_distance;
    struct wlr_fbox box = {.x = screen->x, .y = screen->y};
    zn_screen_get_effective_size(screen, &box.width, &box.height);
    zn_wlr_fbox_closest_point(
        &box, x, y, &current_closest_x, &current_closest_y);
    current_closest_distance =
        pow(x - current_closest_x, 2) + pow(y - current_closest_y, 2);
    if (current_closest_distance < closest_distance) {
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
    struct zn_screen_layout *self, struct zn_screen *new_screen)
{
  wl_list_insert(&self->screen_list, &new_screen->link);
  zn_screen_layout_rearrange(self);
}

void
zn_screen_layout_remove(struct zn_screen_layout *self, struct zn_screen *screen)
{
  wl_list_remove(&screen->link);
  zn_screen_layout_rearrange(self);
}

int
zn_screen_layout_screen_count(struct zn_screen_layout *self)
{
  return wl_list_length(&self->screen_list);
}

struct zn_screen_layout *
zn_screen_layout_create(void)
{
  struct zn_screen_layout *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->screen_list);

  return self;

err:
  return NULL;
}

void
zn_screen_layout_destroy(struct zn_screen_layout *self)
{
  wl_list_remove(&self->screen_list);
  free(self);
}
