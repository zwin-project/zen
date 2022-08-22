#include "zen/scene/screen-layout.h"

#include <float.h>
#include <math.h>

#include "zen-common.h"
#include "zen/scene/screen.h"

// from wlroots::wlr_box_closest_point
static void
wlr_fbox_closest_point(const struct wlr_fbox* box, double x, double y,
    double* dest_x, double* dest_y)
{
  // if box is empty, then it contains no points, so no closest point either
  if (box->width <= 0 || box->height <= 0) {
    *dest_x = NAN;
    *dest_y = NAN;
    return;
  }

  // find the closest x point
  if (x < box->x) {
    *dest_x = box->x;
  } else if (x >= box->x + box->width) {
    *dest_x = box->x + box->width - 1;
  } else {
    *dest_x = x;
  }

  // find closest y point
  if (y < box->y) {
    *dest_y = box->y;
  } else if (y >= box->y + box->height) {
    *dest_y = box->y + box->height - 1;
  } else {
    *dest_y = y;
  }
}

static void
zn_screen_layout_rearrange(struct zn_screen_layout* self)
{
  int x = 0;
  struct zn_screen* screen;
  struct wlr_fbox box;

  wl_list_for_each(screen, &self->screens, link)
  {
    zn_screen_get_fbox(screen, &box);
    screen->x = x;
    screen->y = 0;
    x += box.width;
  }
}

struct zn_screen*
zn_screen_layout_get_closest_screen(struct zn_screen_layout* self, double x,
    double y, double* dst_x, double* dst_y)
{
  double closest_x = 0, closest_y = 0, closest_distance = DBL_MAX;
  struct zn_screen* closest_screen = NULL;
  struct zn_screen* screen;

  wl_list_for_each(screen, &self->screens, link)
  {
    double current_closest_x, current_closest_y, current_closest_distance;
    struct wlr_fbox box;
    zn_screen_get_fbox(screen, &box);
    wlr_fbox_closest_point(&box, x, y, &current_closest_x, &current_closest_y);
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
    struct zn_screen_layout* self, struct zn_screen* new_screen)
{
  wl_list_insert(&self->screens, &new_screen->link);
  zn_screen_layout_rearrange(self);
  wl_signal_emit(&self->events.new_screen, new_screen);

  zn_scene_reassign_boards(self->scene);
}

void
zn_screen_layout_remove(struct zn_screen_layout* self, struct zn_screen* screen)
{
  wl_list_remove(&screen->link);
  zn_screen_layout_rearrange(self);
  zn_scene_reassign_boards(self->scene);
}

struct zn_screen_layout*
zn_screen_layout_create(struct zn_scene* scene)
{
  struct zn_screen_layout* self;
  self = zalloc(sizeof *self);

  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->scene = scene;
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
