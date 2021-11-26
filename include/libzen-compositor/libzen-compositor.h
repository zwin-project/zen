#ifndef LIBZEN_COMPOSIOR_H
#define LIBZEN_COMPOSIOR_H

#include <time.h>
#include <wayland-server.h>

#include "debug.h"
#include "helpers.h"
#include "timespec-util.h"

#ifdef __cplusplus
extern "C" {
#endif

// Dependency
//                                       + ------------------- +
// backend --> renderer -------------->  |                     |
//        \            \---> shell --->  | compositor (libzen) |
//         \-------------------------->  |                     |
//                                       + ------------------- +

struct zen_weak_link {
  struct wl_resource* resource;
  struct wl_listener listener;
};

struct zen_compositor {
  struct wl_display* display;
  struct wl_global* global;

  struct wl_signal frame_signal;

  struct zen_shell_base* shell_base;
  struct zen_backend* backend;

  struct wl_event_source* repaint_timer;
  uint32_t repaint_window_msec;
};

struct zen_virtual_object {
  struct zen_compositor* compositor;

  struct wl_signal commit_signal;

  struct wl_listener compositor_frame_signal_listener;
  struct wl_list frame_callback_list;

  struct {
    struct wl_list frame_callback_list;
  } pending;
};

struct zen_shell_base {
  const char* type;
};

struct zen_output {
  struct timespec frame_time;

  void (*repaint)(struct zen_output* output);
};

struct zen_backend {
  struct zen_output* output;
};

// methods of zen_compositor
struct zen_compositor* zen_compositor_create(struct wl_display* display);

void zen_compositor_destroy(struct zen_compositor* compositor);

int zen_compositor_load_shell(struct zen_compositor* compositor);

int zen_compositor_load_backend(struct zen_compositor* compositor);

void zen_compositor_finish_frame(
    struct zen_compositor* compositor, struct timespec next_repaint);

// methods of zen_weak_link
void zen_weak_link_init(struct zen_weak_link* link);

void zen_weak_link_set(
    struct zen_weak_link* link, struct wl_resource* resource);

void zen_weak_link_unset(struct zen_weak_link* link);

void* zen_weak_link_get_user_data(struct zen_weak_link* link);

// Interfaces below will be implementaed outside of libzen-compositor

// methods of zen_shell;
struct zen_shell_base* zen_shell_create(struct zen_compositor* compositor);

void zen_shell_destroy(struct zen_shell_base* shell);

// methods of zen_backend
struct zen_backend* zen_backend_create(struct zen_compositor* compositor);

void zen_backend_destroy(struct zen_backend* backend);

#ifdef __cplusplus
}
#endif

#endif  //  LIBZEN_COMPOSIOR_H
