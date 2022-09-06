#include "zen/screen-renderer.h"

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_surface.h>

#include "zen-common.h"
#include "zen/input/seat.h"
#include "zen/output.h"
#include "zen/scene/board.h"
#include "zen/scene/screen.h"
#include "zen/scene/view.h"

typedef void (*surface_iterator_func_t)(struct zn_screen *screen,
    struct wlr_surface *surface, struct wlr_renderer *renderer,
    struct wlr_fbox *fbox, pixman_region32_t *screen_damage);

struct surface_iterator_data {
  surface_iterator_func_t iterator;

  struct zn_screen *screen;
  struct zn_view *view;
  struct wlr_renderer *renderer;
  pixman_region32_t *screen_damage;
};

static void
zn_screen_renderer_scissor_output(
    struct zn_output *output, pixman_box32_t *rect)
{
  struct wlr_output *wlr_output = output->wlr_output;
  struct wlr_renderer *renderer = wlr_output->renderer;
  int output_width, output_height;
  enum wl_output_transform transform;
  struct wlr_box box = {
      .x = rect->x1,
      .y = rect->y1,
      .width = rect->x2 - rect->x1,
      .height = rect->y2 - rect->y1,
  };

  wlr_output_transformed_resolution(wlr_output, &output_width, &output_height);

  transform = wlr_output_transform_invert(wlr_output->transform);
  wlr_box_transform(&box, &box, transform, output_width, output_height);

  wlr_renderer_scissor(renderer, &box);
}

static void
zn_screen_renderer_get_surface_fbox(struct wlr_surface *surface,
    struct zn_view *view, int surface_x, int surface_y,
    struct wlr_fbox *surface_box)
{
  surface_box->x = view->surface_x + surface_x;
  surface_box->y = view->surface_y + surface_y;
  surface_box->width = surface->current.width;
  surface_box->height = surface->current.height;
}

static void
zn_screen_renderer_for_each_surface_iterator(
    struct wlr_surface *surface, int surface_x, int surface_y, void *user_data)
{
  struct surface_iterator_data *data = user_data;
  struct wlr_fbox surface_box;

  zn_screen_renderer_get_surface_fbox(
      surface, data->view, surface_x, surface_y, &surface_box);

  data->iterator(
      data->screen, surface, data->renderer, &surface_box, data->screen_damage);
}

static void
zn_screen_renderer_render_surface_iterator(struct zn_screen *screen,
    struct wlr_surface *surface, struct wlr_renderer *renderer,
    struct wlr_fbox *fbox, pixman_region32_t *screen_damage)
{
  struct zn_output *output = screen->output;
  struct wlr_texture *texture = wlr_surface_get_texture(surface);
  struct wlr_box transformed_box;
  pixman_region32_t render_damage;
  pixman_box32_t *rects;
  float matrix[9];
  int rect_count;

  zn_output_box_effective_to_transformed_coords(output, fbox, &transformed_box);

  pixman_region32_init(&render_damage);
  pixman_region32_union_rect(&render_damage, &render_damage, transformed_box.x,
      transformed_box.y, transformed_box.width, transformed_box.height);

  pixman_region32_intersect(&render_damage, &render_damage, screen_damage);

  if (!pixman_region32_not_empty(&render_damage)) {
    goto render_damage_finish;
  }

  wlr_matrix_project_box(matrix, &transformed_box, WL_OUTPUT_TRANSFORM_NORMAL,
      0, output->wlr_output->transform_matrix);

  rects = pixman_region32_rectangles(&render_damage, &rect_count);
  for (int i = 0; i < rect_count; i++) {
    zn_screen_renderer_scissor_output(output, &rects[i]);
    wlr_render_texture_with_matrix(renderer, texture, matrix, 1.0f);
  }

render_damage_finish:
  pixman_region32_fini(&render_damage);
}

static void
zn_screen_renderer_render_view_popups(struct zn_screen *screen,
    struct zn_view *view, struct wlr_renderer *renderer,
    pixman_region32_t *screen_damage)
{
  struct surface_iterator_data data = {
      .iterator = zn_screen_renderer_render_surface_iterator,
      .screen = screen,
      .view = view,
      .renderer = renderer,
      .screen_damage = screen_damage,
  };

  if (view->impl->for_each_popup_surface) {
    view->impl->for_each_popup_surface(
        view, zn_screen_renderer_for_each_surface_iterator, &data);
  }
}

static void
zn_screen_renderer_render_view(struct zn_screen *screen, struct zn_view *view,
    struct wlr_renderer *renderer, pixman_region32_t *screen_damage)
{
  struct wlr_surface *wlr_surface = view->impl->get_wlr_surface(view);
  struct surface_iterator_data data = {
      .iterator = zn_screen_renderer_render_surface_iterator,
      .screen = screen,
      .view = view,
      .renderer = renderer,
      .screen_damage = screen_damage,
  };

  wlr_surface_for_each_surface(
      wlr_surface, zn_screen_renderer_for_each_surface_iterator, &data);

  // TODO: render popups only focused view
  zn_screen_renderer_render_view_popups(screen, view, renderer, screen_damage);
}

static void
zn_screen_renderer_render_cursor(struct zn_screen *screen,
    struct zn_cursor *cursor, struct wlr_renderer *renderer,
    pixman_region32_t *screen_damage)
{
  struct wlr_texture *texture;
  struct wlr_fbox fbox;
  struct wlr_box transformed_box;
  struct zn_output *output = screen->output;
  pixman_region32_t render_damage;
  pixman_box32_t *rects;
  int rect_count;
  float matrix[9];

  if (!cursor->visible || cursor->screen != screen) {
    return;
  }

  zn_cursor_get_fbox(cursor, &fbox);
  zn_output_box_effective_to_transformed_coords(
      output, &fbox, &transformed_box);

  pixman_region32_init(&render_damage);
  pixman_region32_union_rect(&render_damage, &render_damage, transformed_box.x,
      transformed_box.y, transformed_box.width, transformed_box.height);

  pixman_region32_intersect(&render_damage, &render_damage, screen_damage);

  if (!pixman_region32_not_empty(&render_damage)) {
    goto render_damage_finish;
  }

  if (cursor->surface != NULL) {
    texture = wlr_surface_get_texture(cursor->surface);
  } else {
    texture = cursor->xcursor_texture;
  }

  if (texture == NULL) {
    goto render_damage_finish;
  }

  wlr_matrix_project_box(matrix, &transformed_box, WL_OUTPUT_TRANSFORM_NORMAL,
      0, output->wlr_output->transform_matrix);

  rects = pixman_region32_rectangles(&render_damage, &rect_count);
  for (int i = 0; i < rect_count; i++) {
    zn_screen_renderer_scissor_output(screen->output, &rects[i]);
    wlr_render_texture_with_matrix(renderer, texture, matrix, 1.0f);
  }

render_damage_finish:
  pixman_region32_fini(&render_damage);
}

void
zn_screen_renderer_render(struct zn_screen *screen,
    struct wlr_renderer *renderer, pixman_region32_t *damage)
{
  struct zn_view *view;
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->input_manager->seat->cursor;
  struct zn_output *output = screen->output;
  struct zn_board *board = zn_screen_get_current_board(screen);
  struct wlr_output *wlr_output = output->wlr_output;
  pixman_region32_t screen_damage;
  pixman_box32_t *rects;
  int output_width, output_height, rect_count;

  wlr_renderer_begin(renderer, wlr_output->width, wlr_output->height);

  pixman_region32_init(&screen_damage);

  wlr_output_transformed_resolution(wlr_output, &output_width, &output_height);
  pixman_region32_union_rect(
      &screen_damage, &screen_damage, 0, 0, output_width, output_height);
  pixman_region32_intersect(&screen_damage, &screen_damage, damage);

  if (!pixman_region32_not_empty(&screen_damage)) {
    goto out_commit;
  }

  rects = pixman_region32_rectangles(&screen_damage, &rect_count);
  for (int i = 0; i < rect_count; i++) {
    zn_screen_renderer_scissor_output(output, &rects[i]);
    wlr_renderer_clear(renderer, (float[]){0.2, 0.3, 0.2, 1});
  }

  wl_list_for_each(view, &board->view_list, link)
      zn_screen_renderer_render_view(screen, view, renderer, &screen_damage);

  zn_screen_renderer_render_cursor(screen, cursor, renderer, &screen_damage);

out_commit:
  wlr_renderer_end(renderer);
  wlr_output_commit(wlr_output);
  pixman_region32_fini(&screen_damage);
}
