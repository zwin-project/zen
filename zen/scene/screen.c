#include "zen/scene/screen.h"

#include "zen-common.h"
#include "zen/scene/screen-layout.h"
#include "zen/scene/view.h"

struct zn_view *
zn_screen_get_view_at(struct zn_screen *self, double x, double y)
{
  struct wlr_fbox fbox;
  struct zn_view *view;

  wl_list_for_each_reverse(view, &self->views, link)
  {
    struct wlr_box box;
    zn_view_get_fbox(view, &fbox);
    box.x = fbox.x;
    box.y = fbox.y;
    box.width = fbox.width;
    box.height = fbox.height;

    if (wlr_box_contains_point(&box, x, y)) {
      return view;
    }
  }

  return NULL;
}

void
zn_screen_get_screen_layout_coords(
    struct zn_screen *self, int x, int y, int *dst_x, int *dst_y)
{
  *dst_x = self->x + x;
  *dst_y = self->y + y;
}

void
zn_screen_get_box(struct zn_screen *self, struct wlr_box *box)
{
  wlr_output_effective_resolution(
      self->output->wlr_output, &box->width, &box->height);
  box->x = self->x;
  box->y = self->y;
}

struct zn_screen *
zn_screen_create(
    struct zn_screen_layout *screen_layout, struct zn_output *output)
{
  struct zn_screen *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->output = output;
  self->screen_layout = screen_layout;
  wl_list_init(&self->views);
  wl_signal_init(&self->events.destroy);

  zn_screen_layout_add(screen_layout, self);

  return self;

err:
  return NULL;
}

void
zn_screen_destroy(struct zn_screen *self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->views);
  zn_screen_layout_remove(self->screen_layout, self);
  free(self);
}
