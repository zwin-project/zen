#ifndef ZEN_SHELL_H
#define ZEN_SHELL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libzen-compositor/libzen-compositor.h>

extern char* zen_shell_type;
extern char* zen_cuboid_window_role;

struct zen_desktop_api;

struct zen_shell {
  struct zen_shell_base base;
  struct zen_compositor* compositor;

  struct wl_list cuboid_window_list;

  struct wl_global* global;

  struct zen_desktop_api* interface;
};

struct zen_cuboid_window {
  struct zen_shell* shell;
  struct wl_resource* resource;
  struct zen_virtual_object* virtual_object;
  vec3 half_size;
  versor quaternion;

  struct wl_list link;

  struct wl_listener virtual_object_destroy_listener;
  struct wl_listener virtual_object_render_commit_listener;

  struct zen_render_item* render_item;
};

// Interfaces below will be implementaed outside of zen-shell

struct zen_render_item* zen_cuboid_window_render_item_create(
    struct zen_renderer* renderer, struct zen_cuboid_window* cuboid_window);

void zen_cuboid_window_render_item_destroy(struct zen_render_item* render_item);

#ifdef __cplusplus
}
#endif

#endif  //  ZEN_SHELL_H
