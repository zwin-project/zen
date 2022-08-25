#include "zen/render-2d.h"

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_surface.h>

#include "zen/input/seat.h"
#include "zen/output.h"
#include "zen/scene/board.h"
#include "zen/scene/screen.h"
#include "zen/scene/view.h"

typedef void (*surface_iterator_func_t)(struct wlr_surface *surface,
    struct wlr_renderer *renderer, const struct wlr_fbox *box);

struct surface_iterator_data {
  surface_iterator_func_t iterator;

  struct zn_view *view;
  struct wlr_renderer *renderer;
};

static void
zn_render_2d_get_surface_fbox(struct wlr_surface *surface, struct zn_view *view,
    int surface_x, int surface_y, struct wlr_fbox *surface_box)
{
  surface_box->x = view->x + surface_x;
  surface_box->y = view->y + surface_y;
  surface_box->width = surface->current.width;
  surface_box->height = surface->current.height;
}

static void
zn_render_2d_for_each_surface_iterator(
    struct wlr_surface *surface, int surface_x, int surface_y, void *user_data)
{
  struct surface_iterator_data *data = user_data;
  struct wlr_fbox surface_box;

  zn_render_2d_get_surface_fbox(
      surface, data->view, surface_x, surface_y, &surface_box);

  data->iterator(surface, data->renderer, &surface_box);
}

static void
zn_render_2d_render_surface_iterator(struct wlr_surface *surface,
    struct wlr_renderer *renderer, const struct wlr_fbox *box)
{
  struct wlr_texture *texture = wlr_surface_get_texture(surface);
  float transform[9] = {
      1, 0, 0,  //
      0, 1, 0,  //
      0, 0, 1   //
  };

  wlr_render_texture(renderer, texture, transform, box->x, box->y, 1.f);
}

static void
zn_render_2d_view_popups(struct zn_view *view, struct wlr_renderer *renderer)
{
  struct surface_iterator_data data = {
      .iterator = zn_render_2d_render_surface_iterator,
      .view = view,
      .renderer = renderer,
  };

  if (view->impl->for_each_popup_surface) {
    view->impl->for_each_popup_surface(
        view, zn_render_2d_for_each_surface_iterator, &data);
  }
}

static void
zn_render_2d_view(struct zn_view *view, struct wlr_renderer *renderer)
{
  struct wlr_surface *wlr_surface = view->impl->get_wlr_surface(view);
  struct surface_iterator_data data = {
      .iterator = zn_render_2d_render_surface_iterator,
      .view = view,
      .renderer = renderer,
  };

  wlr_surface_for_each_surface(
      wlr_surface, zn_render_2d_for_each_surface_iterator, &data);

  // TODO: render popups only focused view
  zn_render_2d_view_popups(view, renderer);
}

static void
zn_render_2d_cursor(struct zn_cursor *cursor, struct zn_screen *screen,
    struct wlr_renderer *renderer)
{
  struct wlr_texture *texture;
  float transform[9] = {
      1, 0, 0,  //
      0, 1, 0,  //
      0, 0, 1   //
  };

  if (!cursor->visible || cursor->screen != screen) {
    return;
  }

  if (cursor->surface != NULL) {
    texture = wlr_surface_get_texture(cursor->surface);
  } else {
    texture = cursor->texture;
  }

  if (texture != NULL) {
    wlr_render_texture(renderer, texture, transform,
        cursor->x - cursor->hotspot_x, cursor->y - cursor->hotspot_y, 1.f);
  }
}

void
zn_render_2d_screen(struct zn_screen *screen, struct wlr_renderer *renderer)
{
  struct zn_view *view;
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->input_manager->seat->cursor;
  struct zn_board *board = zn_screen_get_current_board(screen);

  wl_list_for_each(view, &board->view_list, link)
      zn_render_2d_view(view, renderer);

  zn_render_2d_cursor(cursor, screen, renderer);
}
