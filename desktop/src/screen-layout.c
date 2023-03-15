#include "zen-desktop/screen-layout.h"

#include <cglm/vec2.h>
#include <math.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-common/wlr/box.h"
#include "zen-desktop/screen.h"
#include "zen/screen.h"

void
zn_screen_layout_get_closest_position(struct zn_screen_layout *self,
    struct zn_desktop_screen *desktop_screen, vec2 position,
    struct zn_desktop_screen **desktop_screen_out, vec2 position_out)
{
  vec2 layout_position = GLM_VEC2_ZERO_INIT;
  double closest_distance = DBL_MAX;  // Manhattan distance
  struct zn_desktop_screen *desktop_screen_iter = NULL;

  zn_desktop_screen_effective_to_layout_coords(
      desktop_screen, position, layout_position);

  *desktop_screen_out = NULL;
  glm_vec2_zero(position_out);

  wl_list_for_each (desktop_screen_iter, &self->desktop_screen_list, link) {
    double current_closest_x = 0;               // layout coords
    double current_closest_y = 0;               // layout coords
    double current_closest_distance = DBL_MAX;  // Manhattan distance

    // layout coords
    struct wlr_fbox screen_fbox = {
        .x = desktop_screen_iter->screen->layout_position[0],
        .y = desktop_screen_iter->screen->layout_position[1],
        .width = desktop_screen_iter->screen->size[0],
        .height = desktop_screen_iter->screen->size[1],
    };

    zn_wlr_fbox_closest_point(&screen_fbox, layout_position[0],
        layout_position[1], &current_closest_x, &current_closest_y);

    current_closest_distance = fabs(current_closest_x - layout_position[0]) +
                               fabs(current_closest_y - layout_position[1]);

    if (current_closest_distance < closest_distance) {
      closest_distance = current_closest_distance;
      glm_vec2_sub((vec2){(float)current_closest_x, (float)current_closest_y},
          desktop_screen_iter->screen->layout_position, position_out);
      *desktop_screen_out = desktop_screen_iter;
    }
  }
}

struct zn_desktop_screen *
zn_screen_layout_get_main_screen(struct zn_screen_layout *self)
{
  // TODO(@Aki-7): It's naive implementation
  if (wl_list_empty(&self->desktop_screen_list)) {
    return NULL;
  }

  struct zn_desktop_screen *desktop_screen =
      zn_container_of(self->desktop_screen_list.next, desktop_screen, link);

  return desktop_screen;
}

void
zn_screen_layout_reposition(struct zn_screen_layout *self)
{
  // TODO(@Aki-7): it's a naive implementation
  vec2 position = GLM_VEC2_ZERO_INIT;
  struct zn_desktop_screen *desktop_screen = NULL;
  wl_list_for_each (desktop_screen, &self->desktop_screen_list, link) {
    zn_desktop_screen_set_position(desktop_screen, position);
    position[0] += desktop_screen->screen->size[0];
  }
}

void
zn_screen_layout_add(
    struct zn_screen_layout *self, struct zn_desktop_screen *desktop_screen)
{
  wl_list_insert(self->desktop_screen_list.prev, &desktop_screen->link);

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

  wl_list_init(&self->desktop_screen_list);

  return self;

err:
  return NULL;
}

void
zn_screen_layout_destroy(struct zn_screen_layout *self)
{
  wl_list_remove(&self->desktop_screen_list);
  free(self);
}
