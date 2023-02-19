#include "output.h"

#include <cglm/types.h>
#include <cglm/vec2.h>
#include <math.h>
#include <pixman.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output_damage.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/screen.h"
#include "zen/snode.h"

static void zn_output_destroy(struct zn_output *self);
static const vec4 screen_background_color = {1.F, 1.F, 1.F, 1.F};

static inline int
scale_length(double length, double offset, double scale)
{
  return (int)(round((offset + length) * scale) - round(offset * scale));
}

/// @param rect is transformed coords
static void
zn_output_scissor(struct zn_output *self, pixman_box32_t *rect)
{
  struct wlr_output *wlr_output = self->wlr_output;
  struct wlr_renderer *renderer = wlr_output->renderer;
  int transformed_width = 0;
  int transformed_height = 0;

  struct wlr_box box = {
      .x = rect->x1,
      .y = rect->y1,
      .width = rect->x2 - rect->x1,
      .height = rect->y2 - rect->y1,
  };

  wlr_output_transformed_resolution(
      wlr_output, &transformed_width, &transformed_height);

  enum wl_output_transform transform =
      wlr_output_transform_invert(wlr_output->transform);
  wlr_box_transform(
      &box, &box, transform, transformed_width, transformed_height);

  wlr_renderer_scissor(renderer, &box);
}

void
zn_output_box_effective_to_transformed_coords(struct zn_output *self,
    struct wlr_fbox *effective, struct wlr_box *transformed)
{
  float scale = self->wlr_output->scale;
  transformed->x = (int)round(effective->x * scale);
  transformed->y = (int)round(effective->y * scale);
  transformed->width = scale_length(effective->width, effective->x, scale);
  transformed->height = scale_length(effective->height, effective->y, scale);
}

/// @param snode is nonnull
static void  // NOLINTNEXTLINE(misc-no-recursion)
zn_output_render_snode(struct zn_output *self, struct zn_snode *snode,
    pixman_region32_t *screen_damage)
{
  struct wlr_texture *texture = zn_snode_get_texture(snode);

  if (texture) {
    struct wlr_fbox effective_box;
    zn_snode_get_fbox(snode, &effective_box);

    struct wlr_box transformed_box;
    zn_output_box_effective_to_transformed_coords(
        self, &effective_box, &transformed_box);

    pixman_region32_t render_damage;
    pixman_region32_init(&render_damage);

    pixman_region32_union_rect(&render_damage, &render_damage,
        transformed_box.x, transformed_box.y, transformed_box.width,
        transformed_box.height);
    pixman_region32_intersect(&render_damage, &render_damage, screen_damage);

    if (pixman_region32_not_empty(&render_damage)) {
      float matrix[9];
      pixman_box32_t *rects = NULL;
      int rect_count = 0;

      wlr_matrix_project_box(matrix, &transformed_box,
          WL_OUTPUT_TRANSFORM_NORMAL, 0, self->wlr_output->transform_matrix);

      rects = pixman_region32_rectangles(&render_damage, &rect_count);
      for (int i = 0; i < rect_count; i++) {
        zn_output_scissor(self, &rects[i]);
        wlr_render_texture_with_matrix(
            self->wlr_output->renderer, texture, matrix, 1.F);
      }
    }

    pixman_region32_fini(&render_damage);
  }

  struct zn_snode *child = NULL;
  wl_list_for_each (child, &snode->child_node_list, link) {
    zn_output_render_snode(self, child, screen_damage);
  }
}

static void
zn_output_render(struct zn_output *self, pixman_region32_t *damage)
{
  pixman_region32_t screen_damage;
  int transformed_width = 0;
  int transformed_height = 0;
  int rect_count = 0;
  pixman_box32_t *rects = NULL;

  pixman_region32_init(&screen_damage);

  wlr_output_transformed_resolution(
      self->wlr_output, &transformed_width, &transformed_height);

  pixman_region32_union_rect(&screen_damage, &screen_damage, 0, 0,
      transformed_width, transformed_height);

  pixman_region32_intersect(&screen_damage, &screen_damage, damage);

  if (!pixman_region32_not_empty(&screen_damage)) {
    goto screen_damage_finish;
  }

  rects = pixman_region32_rectangles(&screen_damage, &rect_count);
  for (int i = 0; i < rect_count; i++) {
    zn_output_scissor(self, &rects[i]);
    wlr_renderer_clear(self->wlr_output->renderer, screen_background_color);
  }

  zn_output_render_snode(self, self->screen->snode_root, &screen_damage);

screen_damage_finish:
  pixman_region32_fini(&screen_damage);
}

static void
zn_output_handle_damage_frame(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_output *self =
      zn_container_of(listener, self, damage_frame_listener);
  struct wlr_renderer *renderer = self->wlr_output->renderer;
  bool needs_frame = false;
  pixman_region32_t damage;

  {  // notify frame
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    zn_screen_notify_frame(self->screen, &now);
  }

  pixman_region32_init(&damage);

  if (!wlr_output_damage_attach_render(self->damage, &needs_frame, &damage)) {
    static bool warned = false;
    if (!warned) {
      zn_warn("wlr_output_damage_attach_render failed");
      goto damage_finish;
    }
  }

  if (needs_frame) {
    wlr_renderer_begin(
        renderer, self->wlr_output->width, self->wlr_output->height);

    zn_output_render(self, &damage);

    wlr_renderer_end(renderer);
    wlr_output_commit(self->wlr_output);
  } else {
    wlr_output_rollback(self->wlr_output);
  }

damage_finish:
  pixman_region32_fini(&damage);
}

static void
zn_output_damage(void *impl_data, struct wlr_fbox *damage_fbox)
{
  struct zn_output *self = impl_data;
  struct wlr_box transformed_box;

  zn_output_box_effective_to_transformed_coords(
      self, damage_fbox, &transformed_box);

  wlr_output_damage_add_box(self->damage, &transformed_box);
}

const struct zn_screen_interface screen_implementation = {
    .damage = zn_output_damage,
};

static void
zn_output_handle_wlr_output_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_output *self =
      zn_container_of(listener, self, wlr_output_destroy_listener);

  zn_output_destroy(self);
}

static void
zn_output_handle_mode(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_output *self = zn_container_of(listener, self, mode_listener);
  vec2 size;  // effective coords
  int width = 0;
  int height = 0;
  wlr_output_transformed_resolution(self->wlr_output, &width, &height);
  size[0] = (float)width / self->wlr_output->scale;
  size[1] = (float)height / self->wlr_output->scale;

  zn_screen_notify_resize(self->screen, size);
}

struct zn_output *
zn_output_create(struct wlr_output *wlr_output)
{
  struct zn_output *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_output = wlr_output;

  self->screen = zn_screen_create(self, &screen_implementation);
  if (self->screen == NULL) {
    zn_error("Failed to create a zn_screen");
    goto err_free;
  }

  self->damage = wlr_output_damage_create(wlr_output);
  if (self->damage == NULL) {
    zn_error("Failed to create a wlr_output_damage");
    goto err_screen;
  }

  struct wlr_output_mode *mode = wlr_output_preferred_mode(self->wlr_output);
  if (mode == NULL) {
    zn_error("Output %p doesn't support modes", (void *)wlr_output);
    goto err_damage;
  }

  self->mode_listener.notify = zn_output_handle_mode;
  wl_signal_add(&self->wlr_output->events.mode, &self->mode_listener);

  self->wlr_output_destroy_listener.notify =
      zn_output_handle_wlr_output_destroy;
  wl_signal_add(
      &self->damage->events.destroy, &self->wlr_output_destroy_listener);

  self->damage_frame_listener.notify = zn_output_handle_damage_frame;
  wl_signal_add(&self->damage->events.frame, &self->damage_frame_listener);

  // initial setup
  wlr_output_set_mode(self->wlr_output, mode);
  wlr_output_enable(self->wlr_output, true);
  if (!wlr_output_commit(self->wlr_output)) {
    zn_error("Failed to commit initial status");
    goto err_signals;
  }

  return self;

err_signals:
  wl_list_remove(&self->damage_frame_listener.link);
  wl_list_remove(&self->wlr_output_destroy_listener.link);
  wl_list_remove(&self->damage_frame_listener.link);

err_damage:
  wlr_output_damage_destroy(self->damage);

err_screen:
  zn_screen_destroy(self->screen);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_output_destroy(struct zn_output *self)
{
  zn_screen_destroy(self->screen);
  wl_list_remove(&self->damage_frame_listener.link);
  wl_list_remove(&self->wlr_output_destroy_listener.link);

  /**
   * Uncommenting this will cause error as wlr_output_damage does not do
   * wl_list_remove(&damage->events.frame)
   */
  // wl_list_remove(&self->damage_frame_listener.link);
  free(self);
}
