#pragma once
#include <cairo.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/util/box.h>

enum zigzag_anchor {
  ZIGZAG_ANCHOR_CENTER,
  ZIGZAG_ANCHOR_LEFT,
  ZIGZAG_ANCHOR_RIGHT,
  ZIGZAG_ANCHOR_TOP,
  ZIGZAG_ANCHOR_BOTTOM,
};

struct zigzag_node;

struct zigzag_layout_impl {
  void (*on_damage)(struct zigzag_node *node);
};

struct zigzag_layout {
  double screen_width;
  double screen_height;

  struct wl_list node_list;  // zigzag_node::link

  void *user_data;

  const struct zigzag_layout_impl *implementation;
};

struct zigzag_layout *zigzag_layout_create(
    const struct zigzag_layout_impl *implementation, double screen_width,
    double screen_height, void *user_data);

void zigzag_layout_destroy(struct zigzag_layout *self);

typedef bool (*zigzag_node_render_t)(struct zigzag_node *self, cairo_t *cr);

struct zigzag_node_impl {
  void (*on_click)(struct zigzag_node *self, double x, double y);
  void (*set_frame)(
      struct zigzag_node *self, double screen_width, double screen_height);
  zigzag_node_render_t render;
};

struct zigzag_node {
  struct zigzag_layout *layout;

  struct wlr_fbox frame;
  struct wlr_texture *texture;  // nonnull

  void *user_data;

  // for zigzag_layout::node_list or zigzag_node::node_list
  struct wl_list link;

  // zigzag_node::link
  struct wl_list node_list;

  bool visible;

  const struct zigzag_node_impl *implementation;
};

struct zigzag_node *zigzag_node_create(
    const struct zigzag_node_impl *implementation, struct zigzag_layout *layout,
    struct wlr_renderer *renderer, bool visible, void *user_data);

void zigzag_node_destroy(struct zigzag_node *self);

cairo_surface_t *zigzag_node_render_cairo_surface(struct zigzag_node *self,
    zigzag_node_render_t render, double width, double height);

void zigzag_node_update_texture(
    struct zigzag_node *self, struct wlr_renderer *renderer);

bool zigzag_node_contains_point(struct zigzag_node *self, double x, double y);

void zigzag_node_show_texture_with_matrix(struct zigzag_node *self,
    struct wlr_renderer *renderer, const float matrix[static 9]);

void zigzag_node_hide(struct zigzag_node *self);
void zigzag_node_show(struct zigzag_node *self);

struct wlr_texture *zigzag_wlr_texture_from_cairo_surface(
    cairo_surface_t *surface, struct wlr_renderer *renderer);

void zigzag_cairo_draw_text(cairo_t *cr, char *text, double x, double y,
    enum zigzag_anchor horizontal_anchor, enum zigzag_anchor vertical_anchor);

void zigzag_cairo_draw_rounded_rectangle(cairo_t *cr, double x, double y,
    double width, double height, double radius);

bool zigzag_cairo_stamp_svg_on_surface(cairo_t *cr, const char *filename,
    double x, double y, double width, double height);

void zigzag_cairo_draw_rounded_bubble(cairo_t *cr, double x, double y,
    double width, double height, double radius, double tip_x);
