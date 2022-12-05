#include "zen/screen/renderer.h"

#include <wlr/types/wlr_matrix.h>

#include "zen/board.h"
#include "zen/cursor.h"
#include "zen/screen/output.h"
#include "zen/server.h"
#include "zen/view.h"

static void
scissor_output(struct zn_output *output, pixman_box32_t *rect)
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

void
render_background(struct zn_output *output, struct wlr_renderer *renderer,
    pixman_region32_t *screen_damage)
{
  struct zn_server *server = zn_server_get_singleton();
  pixman_box32_t *rects;
  int rect_count;

  float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  if (output->screen->board &&
      server->display_system == ZN_DISPLAY_SYSTEM_SCREEN) {
    color[0] = output->screen->board->color[0];
    color[1] = output->screen->board->color[1];
    color[2] = output->screen->board->color[2];
  }

  rects = pixman_region32_rectangles(screen_damage, &rect_count);
  for (int i = 0; i < rect_count; i++) {
    scissor_output(output, &rects[i]);
    wlr_renderer_clear(renderer, color);
  }
}

void
render_cursor(struct zn_output *output, struct zn_cursor *cursor,
    struct wlr_renderer *renderer, pixman_region32_t *screen_damage)
{
  struct wlr_texture *cursor_texture = zn_cursor_get_texture(cursor);
  struct wlr_fbox cursor_fbox;
  struct wlr_box transformed_box;
  pixman_region32_t render_damage;
  pixman_box32_t *rects;
  int rect_count;

  if (!cursor_texture || !cursor->board ||
      cursor->board->screen != output->screen) {
    return;
  }

  zn_cursor_get_fbox(cursor, &cursor_fbox);
  zn_output_box_effective_to_transformed_coords(
      output, &cursor_fbox, &transformed_box);

  pixman_region32_init(&render_damage);
  pixman_region32_union_rect(&render_damage, &render_damage, transformed_box.x,
      transformed_box.y, transformed_box.width, transformed_box.height);
  pixman_region32_intersect(&render_damage, &render_damage, screen_damage);

  if (pixman_region32_not_empty(&render_damage)) {
    float matrix[9];
    wlr_matrix_project_box(matrix, &transformed_box, WL_OUTPUT_TRANSFORM_NORMAL,
        0, output->wlr_output->transform_matrix);
    rects = pixman_region32_rectangles(&render_damage, &rect_count);
    for (int i = 0; i < rect_count; i++) {
      scissor_output(output, &rects[i]);
      wlr_render_texture_with_matrix(renderer, cursor_texture, matrix, 1.f);
    }
  }

  pixman_region32_fini(&render_damage);
}

void
render_view(struct zn_output *output, struct zn_view *view,
    struct wlr_renderer *renderer, pixman_region32_t *screen_damage)
{
  struct wlr_texture *texture = wlr_surface_get_texture(view->surface);
  struct wlr_fbox view_fbox;
  struct wlr_box transformed_box;
  pixman_region32_t render_damage;
  pixman_box32_t *rects;
  int rect_count;

  if (!texture) return;

  zn_view_get_surface_fbox(view, &view_fbox);
  zn_output_box_effective_to_transformed_coords(
      output, &view_fbox, &transformed_box);

  pixman_region32_init(&render_damage);
  pixman_region32_union_rect(&render_damage, &render_damage, transformed_box.x,
      transformed_box.y, transformed_box.width, transformed_box.height);
  pixman_region32_intersect(&render_damage, &render_damage, screen_damage);

  if (pixman_region32_not_empty(&render_damage)) {
    float matrix[9];
    wlr_matrix_project_box(matrix, &transformed_box, WL_OUTPUT_TRANSFORM_NORMAL,
        0, output->wlr_output->transform_matrix);
    rects = pixman_region32_rectangles(&render_damage, &rect_count);
    for (int i = 0; i < rect_count; i++) {
      scissor_output(output, &rects[i]);
      wlr_render_texture_with_matrix(renderer, texture, matrix, 1.f);
    }
  }
}

void
zn_screen_renderer_render(struct zn_output *output,
    struct wlr_renderer *renderer, pixman_region32_t *damage)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_board *board = output->screen->board;
  pixman_region32_t screen_damage;
  int output_width, output_height;

  pixman_region32_init(&screen_damage);

  wlr_output_transformed_resolution(
      output->wlr_output, &output_width, &output_height);
  pixman_region32_union_rect(
      &screen_damage, &screen_damage, 0, 0, output_width, output_height);
  pixman_region32_intersect(&screen_damage, &screen_damage, damage);

  if (!pixman_region32_not_empty(&screen_damage)) {
    goto out;
  }

  render_background(output, renderer, &screen_damage);

  if (server->display_system != ZN_DISPLAY_SYSTEM_SCREEN) goto out;

  if (board) {
    struct zn_view *view;
    wl_list_for_each (view, &board->view_list, board_link)
      render_view(output, view, renderer, &screen_damage);
  }

  render_cursor(output, server->scene->cursor, renderer, &screen_damage);

out:
  pixman_region32_fini(&screen_damage);
}
