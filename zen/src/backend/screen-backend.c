#include "screen-backend.h"

#include <cglm/vec2.h>

#include "layer-surface.h"
#include "output.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/snode.h"

static void
apply_exclusive(struct wlr_box *usable_area, uint32_t anchor, int32_t exclusive,
    uint32_t margin_top, uint32_t margin_right, uint32_t margin_bottom,
    uint32_t margin_left)
{
  if (exclusive <= 0) {
    return;
  }
  struct {
    uint32_t singular_anchor;
    uint32_t anchor_triplet;
    int *positive_axis;
    int *negative_axis;
    int margin;
  } edges[] = {
      // Top
      {
          .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP,
          .anchor_triplet = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                            ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                            ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP,
          .positive_axis = &usable_area->y,
          .negative_axis = &usable_area->height,
          .margin = (int)margin_top,
      },
      // Bottom
      {
          .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
          .anchor_triplet = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                            ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                            ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
          .positive_axis = NULL,
          .negative_axis = &usable_area->height,
          .margin = (int)margin_bottom,
      },
      // Left
      {
          .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT,
          .anchor_triplet = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                            ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                            ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
          .positive_axis = &usable_area->x,
          .negative_axis = &usable_area->width,
          .margin = (int)margin_left,
      },
      // Right
      {
          .singular_anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
          .anchor_triplet = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                            ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                            ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM,
          .positive_axis = NULL,
          .negative_axis = &usable_area->width,
          .margin = (int)margin_right,
      },
  };
  for (size_t i = 0; i < sizeof(edges) / sizeof(edges[0]); ++i) {
    if ((anchor == edges[i].singular_anchor ||
            anchor == edges[i].anchor_triplet) &&
        exclusive + edges[i].margin > 0) {
      if (edges[i].positive_axis) {
        *edges[i].positive_axis += exclusive + edges[i].margin;
      }
      if (edges[i].negative_axis) {
        *edges[i].negative_axis -= exclusive + edges[i].margin;
      }
      break;
    }
  }
}

static struct wlr_box
calculate_surface_box(
    struct wlr_layer_surface_v1_state *state, struct wlr_box bounds)
{
  struct wlr_box box = {
      .width = (int)state->desired_width,
      .height = (int)state->desired_height,
  };

  // Horizontal axis
  const uint32_t left_right =
      ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
  if (box.width == 0) {
    box.x = bounds.x;
  } else if ((state->anchor & left_right) == left_right) {
    box.x = bounds.x + ((bounds.width / 2) - (box.width / 2));
  } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) {
    box.x = bounds.x;
  } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
    box.x = bounds.x + (bounds.width - box.width);
  } else {
    box.x = bounds.x + ((bounds.width / 2) - (box.width / 2));
  }

  // Vertical axis
  const uint32_t top_bottom =
      ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
  if (box.height == 0) {
    box.y = bounds.y;
  } else if ((state->anchor & top_bottom) == top_bottom) {
    box.y = bounds.y + ((bounds.height / 2) - (box.height / 2));
  } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
    box.y = bounds.y;
  } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) {
    box.y = bounds.y + (bounds.height - box.height);
  } else {
    box.y = bounds.y + ((bounds.height / 2) - (box.height / 2));
  }

  // Apply horizontal margin
  if (box.width == 0) {
    box.x += (int)state->margin.left;
    box.width = bounds.width - (int)(state->margin.left + state->margin.right);
  } else if ((state->anchor & left_right) == left_right) {
    // dont' apply margins
  } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) {
    box.x += (int)state->margin.left;
  } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
    box.x -= (int)state->margin.right;
  }

  // Apply vertical margin
  if (box.height == 0) {
    box.y += (int)state->margin.top;
    box.height =
        bounds.height - (int)(state->margin.top + state->margin.bottom);
  } else if ((state->anchor & top_bottom) == top_bottom) {
    // dont' apply margins
  } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
    box.y += (int)state->margin.top;
  } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) {
    box.y -= (int)state->margin.bottom;
  }

  return box;
}

static void
zn_screen_backend_arrange_layer(struct zn_screen_backend *self,
    enum zwlr_layer_shell_v1_layer layer, struct wlr_box *usable_area,
    bool exclusive)
{
  struct zn_snode *layer_node = self->layers[layer];
  struct zn_snode *child = NULL;
  struct wlr_box full_area = {0};
  wlr_output_effective_resolution(
      self->output->wlr_output, &full_area.width, &full_area.height);

  wl_list_for_each (child, &layer_node->child_node_list, link) {
    if (!zn_assert(child->impl == &zn_layer_surface_snode_implementation,
            "Invalid layer snode")) {
      continue;
    }

    struct zn_layer_surface *layer_surface = child->user_data;
    struct wlr_layer_surface_v1_state *state =
        &layer_surface->wlr_layer_surface->current;

    if (exclusive != (state->exclusive_zone > 0)) {
      continue;
    }

    struct wlr_box bounds;
    if (state->exclusive_zone == -1) {
      bounds = full_area;
    } else {
      bounds = *usable_area;
    }

    struct wlr_box box = calculate_surface_box(state, bounds);

    if (!zn_assert(box.width >= 0 && box.height >= 0,
            "Expected layer surface to have positive size")) {
      continue;
    }

    apply_exclusive(usable_area, state->anchor, state->exclusive_zone,
        state->margin.top, state->margin.right, state->margin.bottom,
        state->margin.left);

    wlr_layer_surface_v1_configure(
        layer_surface->wlr_layer_surface, box.width, box.height);

    zn_snode_set_position(
        child, layer_node, (vec2){(float)box.x, (float)box.y});
  }
}

static void
zn_screen_backend_arrange_layers(struct zn_screen_backend *self)
{
  struct wlr_box usable_area = {0};
  wlr_output_effective_resolution(
      self->output->wlr_output, &usable_area.width, &usable_area.height);

  // Arrange exclusive surfaces from top to bottom
  for (int i = 3; i >= 0; i--) {
    zn_screen_backend_arrange_layer(self, i, &usable_area, true);
  }

  // TODO(@Aki-7): Notify screen the usable area change

  // Arrange non-exclusive surfaces from top to bottom
  for (int i = 3; i >= 0; i--) {
    zn_screen_backend_arrange_layer(self, i, &usable_area, false);
  }

  // TODO(@Aki-7): Configure keyboard interactivity
}

void
zn_screen_backend_add_layer_surface(struct zn_screen_backend *self,
    struct zn_layer_surface *layer_surface,
    enum zwlr_layer_shell_v1_layer layer)
{
  zn_snode_set_position(
      layer_surface->snode, self->layers[layer], GLM_VEC2_ZERO);

  zn_screen_backend_arrange_layers(self);
}

struct zn_snode *
zn_screen_backend_get_layer(
    struct zn_screen_backend *self, enum zwlr_layer_shell_v1_layer layer)
{
  return self->layers[layer];
}

struct zn_screen_backend *
zn_screen_backend_create(struct zn_output *output)
{
  struct zn_screen_backend *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->output = output;

  for (int i = 0; i < 4; i++) {
    self->layers[i] = zn_snode_create(self, &zn_snode_noop_implementation);
    if (self->layers[i] == NULL) {
      zn_error("Failed to create a snode");
      goto err_layers;
    }
  }

  return self;

err_layers:
  for (int i = 0; i < 4; i++) {
    if (self->layers[i]) {
      zn_snode_destroy(self->layers[i]);
    }
  }

  free(self);

err:
  return NULL;
}

void
zn_screen_backend_destroy(struct zn_screen_backend *self)
{
  for (int i = 0; i < 4; i++) {
    zn_snode_destroy(self->layers[i]);
  }
  free(self);
}
