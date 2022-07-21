#include "zen/render-2d.h"

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_surface.h>

#include "zen/output.h"
#include "zen/scene/screen.h"
#include "zen/scene/view.h"

static void
zn_render_2d_view(struct zn_view *view, struct wlr_renderer *renderer)
{
  struct wlr_surface *wlr_surface = view->impl->get_wlr_surface(view);
  struct wlr_texture *texture = wlr_surface_get_texture(wlr_surface);

  float transform[9] = {
      1, 0, 0,  //
      0, 1, 0,  //
      0, 0, 1   //
  };

  wlr_render_texture(renderer, texture, transform, 0, 0, 1);
}

void
zn_render_2d_screen(struct zn_screen *screen, struct wlr_renderer *renderer)
{
  struct zn_view *view;
  wl_list_for_each(view, &screen->views, link)
      zn_render_2d_view(view, renderer);
}
