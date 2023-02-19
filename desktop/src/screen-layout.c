#include "zen-desktop/screen-layout.h"

#include <cglm/vec2.h>
#include <math.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-common/wlr/box.h"
#include "zen-desktop/screen-container.h"
#include "zen/screen.h"

/// @param screen : nonnull
static void
zn_screen_layout_effective_to_layout_coords(struct zn_screen_layout *self,
    struct zn_screen *screen, vec2 effective, vec2 layout)
{
  struct zn_screen_container *container = NULL;
  wl_list_for_each (container, &self->screen_container_list, link) {
    if (container->screen == screen) {
      glm_vec2_add(container->position, effective, layout);
      return;
    }
  }

  zn_assert(false, "All screen must be in the screen layout");
}

void
zn_screen_layout_get_closest_position(struct zn_screen_layout *self,
    struct zn_screen *screen, vec2 position, struct zn_screen **screen_out,
    vec2 position_out)
{
  vec2 layout_position = GLM_VEC2_ZERO_INIT;
  double closest_distance = DBL_MAX;  // Manhattan distance
  struct zn_screen_container *container = NULL;

  zn_screen_layout_effective_to_layout_coords(
      self, screen, position, layout_position);

  *screen_out = NULL;
  glm_vec2_zero(position_out);

  wl_list_for_each (container, &self->screen_container_list, link) {
    double current_closest_x = 0;               // layout coords
    double current_closest_y = 0;               // layout coords
    double current_closest_distance = DBL_MAX;  // Manhattan distance

    // layout coords
    struct wlr_fbox container_fbox = {
        .x = container->position[0],
        .y = container->position[1],
        .width = container->screen->size[0],
        .height = container->screen->size[1],
    };

    zn_wlr_fbox_closest_point(&container_fbox, layout_position[0],
        layout_position[1], &current_closest_x, &current_closest_y);

    current_closest_distance = fabs(current_closest_x - layout_position[0]) +
                               fabs(current_closest_y - layout_position[1]);

    if (current_closest_distance < closest_distance) {
      closest_distance = current_closest_distance;
      glm_vec2_sub((vec2){(float)current_closest_x, (float)current_closest_y},
          container->position, position_out);
      *screen_out = container->screen;
    }
  }
}

struct zn_screen *
zn_screen_layout_get_main_screen(struct zn_screen_layout *self)
{
  // TODO(@Aki-7): It's naive implementation
  if (wl_list_empty(&self->screen_container_list)) {
    return NULL;
  }

  struct zn_screen_container *container =
      zn_container_of(self->screen_container_list.next, container, link);

  return container->screen;
}

void
zn_screen_layout_reposition(struct zn_screen_layout *self)
{
  // TODO(@Aki-7): it's a naive implementation
  float x = 0;
  struct zn_screen_container *container = NULL;
  wl_list_for_each (container, &self->screen_container_list, link) {
    container->position[0] = x;
    container->position[1] = 0;
    x += container->screen->size[0];
  }
}

void
zn_screen_layout_add(
    struct zn_screen_layout *self, struct zn_screen_container *container)
{
  wl_list_insert(self->screen_container_list.prev, &container->link);

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

  wl_list_init(&self->screen_container_list);

  return self;

err:
  return NULL;
}

void
zn_screen_layout_destroy(struct zn_screen_layout *self)
{
  wl_list_remove(&self->screen_container_list);
  free(self);
}
