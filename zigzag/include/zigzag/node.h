#pragma once
#include <cairo.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/util/box.h>

struct zigzag_node;

typedef bool (*zigzag_node_render_t)(struct zigzag_node *self, cairo_t *cr);

struct zigzag_edge_size {
  double left, right;
  double top, bottom;
};

struct zigzag_color {
  double r, g, b, a;
};

struct zigzag_node_impl {
  void (*on_click)(struct zigzag_node *self, double x, double y);
  zigzag_node_render_t render;
};

enum zigzag_reconfigure_direction {
  ZIGZAG_RECONFIGURE_HORIZONTAL,
  ZIGZAG_RECONFIGURE_VERTICAL,
};

enum zigzag_reconfigure_type {
  ZIGZAG_RECONFIGURE_START = 0, /** left or top*/
  ZIGZAG_RECONFIGURE_CENTER,
  ZIGZAG_RECONFIGURE_END,     /** right or left*/
  ZIGZAG_RECONFIGURE_JUSTIFY, /** space-between */
  ZIGZAG_RECONFIGURE_TYPE_COUNT,
};

struct zigzag_node {
  struct zigzag_layout *layout;

  struct wlr_fbox frame;
  // nonnull after being inserted to layout or another node
  struct wlr_texture *texture;

  struct zigzag_edge_size padding;
  struct zigzag_edge_size margin;

  struct {
    struct wlr_fbox frame;
  } pending;

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
    bool visible, void *user_data);

void zigzag_node_destroy(struct zigzag_node *self);

cairo_surface_t *zigzag_node_render_cairo_surface(struct zigzag_node *self,
    zigzag_node_render_t render, double width, double height);

void zigzag_node_reconfigure(struct zigzag_node *self,
    enum zigzag_reconfigure_direction direction,
    enum zigzag_reconfigure_type type);

void zigzag_node_update_texture(
    struct zigzag_node *self, struct wlr_renderer *renderer);

void zigzag_node_add_child(struct zigzag_node *parent,
    struct zigzag_node *child, struct wlr_renderer *renderer);

void zigzag_node_child_total_size(
    struct zigzag_node *parent, double *width, double *height);

void zigzag_node_update_frame(struct zigzag_node *self);

bool zigzag_node_contains_point(struct zigzag_node *self, double x, double y);

void zigzag_node_show_texture_with_matrix(struct zigzag_node *self,
    struct wlr_renderer *renderer, const float matrix[static 9]);

void zigzag_node_hide(struct zigzag_node *self);

void zigzag_node_show(struct zigzag_node *self);
