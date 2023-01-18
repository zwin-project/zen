#pragma once
#include <wayland-server-core.h>
#include <wlr/render/wlr_renderer.h>

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

void zigzag_layout_add_node(struct zigzag_layout *layout,
    struct zigzag_node *node, struct wlr_renderer *renderer);

struct zigzag_layout *zigzag_layout_create(
    const struct zigzag_layout_impl *implementation, double screen_width,
    double screen_height, void *user_data);

void zigzag_layout_destroy(struct zigzag_layout *self);
