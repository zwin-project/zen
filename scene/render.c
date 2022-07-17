#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "output.h"
#include "toplevel-view.h"
#include "zen-common.h"
#include "zen-scene.h"

static void
zn_scene_render_toplevel_view(
    struct zn_scene_toplevel_view* toplevel, struct wlr_renderer* renderer)
{
  struct wlr_surface* wlr_surface = toplevel->wlr_xdg_toplevel->base->surface;
  struct wlr_texture* texture = wlr_surface_get_texture(wlr_surface);

  float transform[9] = {
      1, 0, 0,  //
      0, 1, 0,  //
      0, 0, 1   //
  };

  wlr_render_texture(renderer, texture, transform, 0, 0, 1);
}

void
zn_scene_render_output(struct zn_scene_output* output)
{
  struct zn_scene_toplevel_view* toplevel;
  struct wlr_renderer* renderer = output->wlr_output->renderer;
  wl_list_for_each(toplevel, &output->toplevels, link)
      zn_scene_render_toplevel_view(toplevel, renderer);
}
