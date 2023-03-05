#include "zen-common/wlr/box.h"

void
zn_wlr_fbox_closest_point(const struct wlr_fbox *box, double x, double y,
    double *dest_x, double *dest_y)
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
    *dest_x = box->x + box->width;
  } else {
    *dest_x = x;
  }

  // find closest y point
  if (y < box->y) {
    *dest_y = box->y;
  } else if (y >= box->y + box->height) {
    *dest_y = box->y + box->height;
  } else {
    *dest_y = y;
  }
}

bool
zn_wlr_fbox_contains_point(const struct wlr_fbox *box, double x, double y)
{
  if (box->width <= 0 || box->height <= 0) {
    return false;
  }

  return x >= box->x && x < box->x + box->width && y >= box->y &&
         y < box->y + box->height;
}
