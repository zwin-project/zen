#include "zen/scene/view-child.h"

#include <wlr/types/wlr_output_damage.h>

#include "zen/output.h"

// static void
// zn_view_child_get_surface_fbox(
//     struct zn_view_child *self, struct wlr_fbox *fbox)
// {
//   (void)self;
//   (void)fbox;
// }

static void
zn_view_child_add_damage_fbox(
    struct zn_view_child *self, struct wlr_fbox *effective_box)
{
  if (self->view == NULL || self->view->board == NULL ||
      self->view->board->screen == NULL)
    return;

  zn_output_add_damage_box(self->view->board->screen->output, effective_box);
}

void
zn_view_child_damage(struct zn_view_child *self)
{
  struct wlr_surface *surface = self->impl->get_wlr_surface(self);
  struct wlr_fbox damage_box;
  pixman_region32_t damage;
  pixman_box32_t *rects;
  int rect_count;

  pixman_region32_init(&damage);

  wlr_surface_get_effective_damage(surface, &damage);
  rects = pixman_region32_rectangles(&damage, &rect_count);

  for (int i = 0; i < rect_count; ++i) {
    damage_box = (struct wlr_fbox){
        .x = rects[i].x1,
        //.x = self->x + rects[i].x1, // TODO:,
        //ここが上手く働かないなら、parentのview_childのxを足す？ or
        // viewのxを足す
        .y = rects[i].y1,
        //.y = self->y + rects[i].y1,
        .width = rects[i].x2 - rects[i].x1,
        .height = rects[i].y2 - rects[i].y1,
    };
    zn_view_child_add_damage_fbox(self, &damage_box);
  };

  pixman_region32_fini(&damage);
}

void
zn_view_child_damage_whole(struct zn_view_child *self)
{
  (void)self;
}

void
zn_view_child_init(struct zn_view_child *self, struct zn_view *view)
{
  self->view = view;
  self->parent = NULL;
}
