#ifndef ZEN_SCREEN_H
#define ZEN_SCREEN_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_surface.h>

#include "zen/output.h"
#include "zen/scene/screen-layout.h"

typedef void (*zn_screen_for_each_visible_surface_callback_t)(
    struct wlr_surface *surface, void* data);

struct zn_screen {
  int x, y;
  struct zn_output *output;  // zn_output owns zn_screen, nonnull
  struct zn_screen_layout *screen_layout;

  // List of mapped zn_view in z-order, from bottom to top
  struct wl_list views;  // zn_view::link;

  struct wl_list link;  // zn_screen_layout::screens;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

void zn_screen_for_each_visible_surface(struct zn_screen *self,
    zn_screen_for_each_visible_surface_callback_t callback, void* data);

struct zn_view *zn_screen_get_view_at(
    struct zn_screen *self, double x, double y, double *view_x, double *view_y);

void zn_screen_get_screen_layout_coords(
    struct zn_screen *self, int x, int y, int *dst_x, int *dst_y);

void zn_screen_get_box(struct zn_screen *self, struct wlr_box *box);

struct zn_screen *zn_screen_create(
    struct zn_screen_layout *screen_layout, struct zn_output *output);

void zn_screen_destroy(struct zn_screen *self);

#endif  //  ZEN_SCREEN_H
