#ifndef ZEN_SHELL_H
#define ZEN_SHELL_H

#include <libzen-compositor/libzen-compositor.h>

extern char* zen_shell_type;

struct zen_shell {
  struct zen_shell_base base;
  struct zen_compositor* compositor;

  struct wl_global* global;
};

struct zen_cuboid_window {
  struct zen_shell* shell;
  struct wl_resource* resource;
  struct zen_virtual_object* virtual_object;
  vec3 half_size;
  mat4 model_matrix;

  struct zen_render_item* render_item;
};

// Interfaces below will be implementaed outside of zen-shell

struct zen_render_item* zen_cuboid_window_render_item_create(
    struct zen_renderer* renderer, struct zen_cuboid_window* cuboid_window);

void zen_cuboid_window_render_item_destroy(struct zen_render_item* render_item);

#endif  //  ZEN_SHELL_H
