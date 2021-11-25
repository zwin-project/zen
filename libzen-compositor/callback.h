#ifndef LIBZEN_COMPOSIOR_CALLBACK_H
#define LIBZEN_COMPOSIOR_CALLBACK_H

#include <wayland-server.h>

struct zen_callback {
  struct wl_resource *resource;
  struct wl_list link;
};

struct zen_callback *zen_callback_create(struct wl_client *client, uint32_t id);

void zen_callback_done(struct zen_callback *callback, uint32_t callback_data);

#endif  //  LIBZEN_COMPOSIOR_CALLBACK_H
