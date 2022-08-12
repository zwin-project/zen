#include "zen/render-2d.h"

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_surface.h>

#include "zen/output.h"
#include "zen/scene/screen.h"
#include "zen/scene/view.h"
#include "zen/seat.h"

static void
zn_render_2d_view(struct zn_view *view, struct wlr_renderer *renderer)
{
  struct wlr_surface *wlr_surface = view->impl->get_wlr_surface(view);
  struct wlr_texture *texture = wlr_surface_get_texture(wlr_surface);
  struct wlr_fbox fbox;

  float transform[9] = {
      1, 0, 0,  //
      0, 1, 0,  //
      0, 0, 1   //
  };

  zn_view_get_fbox(view, &fbox);
  wlr_render_texture(renderer, texture, transform, fbox.x, fbox.y, 1);
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

  if (cursor->screen == screen && cursor->texture) {
    if (cursor->surface != NULL) {
      texture = wlr_surface_get_texture(cursor->surface);
    } else {
      texture = cursor->texture;
    }
    wlr_render_texture(renderer, texture, transform, cursor->x, cursor->y, 1.f);
  }
}

void
zn_render_2d_screen(struct zn_screen *screen, struct wlr_renderer *renderer)
{
  struct zn_view *view;
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->input_manager->seat->cursor;

  wl_list_for_each(view, &screen->views, link)
      zn_render_2d_view(view, renderer);

  zn_render_2d_cursor(cursor, screen, renderer);
}
