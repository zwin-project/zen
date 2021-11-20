#ifndef LIBZEN_VIRTUAL_OBJECT_H
#define LIBZEN_VIRTUAL_OBJECT_H

#include <wayland-client.h>

#include "compositor.h"

struct zen_virtual_object {
  struct zen_compositor *compositor;

  struct wl_signal commit_signal;

  struct wl_listener compositor_frame_signal_listener;
  struct wl_list frame_callback_list;

  struct {
    struct wl_list frame_callback_list;
  } pending;
};

struct zen_virtual_object *zen_virtual_object_create(
    struct wl_client *client, uint32_t id, struct zen_compositor *compositor);

#endif  //  LIBZEN_VIRTUAL_OBJECT_H
