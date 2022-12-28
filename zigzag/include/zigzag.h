#pragma once
#include <cairo.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/util/box.h>

struct zigzag_node;

struct zigzag_layout_impl {
  void (*on_damage)(struct zigzag_node *node);
};

struct zigzag_layout {
  int output_width;
  int output_height;

  struct wl_list node_list;  // zigzag_node::link

  void *user_data;

  const struct zigzag_layout_impl *implementation;
};

struct zigzag_layout *zigzag_layout_create(
    const struct zigzag_layout_impl *implementation, int output_width,
    int output_height, void *user_data);

void zigzag_layout_destroy(struct zigzag_layout *self);

struct zigzag_node_impl {
  void (*on_click)(struct zigzag_node *self, double x, double y);
  void (*set_frame)(
      struct zigzag_node *self, int output_width, int output_height);
  void (*render)(struct zigzag_node *self, cairo_t *cr);
};

struct zigzag_node {
  struct zigzag_layout *layout;

  struct wlr_box *frame;
  struct wlr_texture *texture;

  void *user_data;

  // for zigzag_layout::node_list or zigzag_node::node_list
  struct wl_list link;

  // zigzag_node::link
  struct wl_list node_list;

  const struct zigzag_node_impl *implementation;
};

struct zigzag_node *zigzag_node_create(
    const struct zigzag_node_impl *implementation, struct zigzag_layout *layout,
    struct wlr_renderer *renderer, void *user_data);

void zigzag_node_destroy(struct zigzag_node *self);

void zigzag_node_update_texture(
    struct zigzag_node *self, struct wlr_renderer *renderer);

struct wlr_texture *zigzag_wlr_texture_from_cairo_surface(
    cairo_surface_t *surface, struct wlr_renderer *renderer);

void zigzag_cairo_draw_rounded_rectangle(
    cairo_t *cr, double width, double height, double radius);

void zigzag_cairo_draw_centered_text(
    cairo_t *cr, char *text, double width, double height);

void zigzag_cairo_draw_left_aligned_text(
    cairo_t *cr, char *text, double width, double height, double padding);

void zigzag_cairo_draw_right_aligned_text(
    cairo_t *cr, char *text, double width, double height, double padding);

void zigzag_cairo_draw_rounded_rectangle(
    cairo_t *cr, double width, double height, double radius);
