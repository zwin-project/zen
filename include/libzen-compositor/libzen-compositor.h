#ifndef LIBZEN_COMPOSIOR_H
#define LIBZEN_COMPOSIOR_H

#include <cglm/cglm.h>
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

  struct zen_seat* seat;
  struct zen_shell_base* shell_base;
  struct zen_renderer* renderer;
  struct zen_backend* backend;

  struct wl_event_source* repaint_timer;
  uint32_t repaint_window_msec;
};

struct zen_virtual_object {
  struct zen_compositor* compositor;

  char* role;
  void* role_object;

  struct wl_signal commit_signal;
  struct wl_signal destroy_signal;

  struct wl_listener compositor_frame_signal_listener;
  struct wl_list frame_callback_list;

  struct {
    struct wl_list frame_callback_list;
  } pending;
};

struct zen_seat {
  struct wl_global* global;

  char* seat_name;
};

struct zen_pointer_motion_event {
  vec3 delta_origin;
  float delta_polar_angle;
  float delta_azimuthal_angle;
};

struct zen_shell_base {
  const char* type;
};

struct zen_renderer {
  const char* type;
};

struct zen_render_item {
  void (*commit)(struct zen_render_item* render_item);
};

struct zen_output {
  struct timespec frame_time;

  void (*repaint)(struct zen_output* output);
};

struct zen_backend {
  struct zen_output* output;
};

struct zen_udev_seat;

// methods of zen_compositor
struct zen_compositor* zen_compositor_create(struct wl_display* display);

void zen_compositor_destroy(struct zen_compositor* compositor);

int zen_compositor_load_shell(struct zen_compositor* compositor);

int zen_compositor_load_renderer(struct zen_compositor* compositor);

int zen_compositor_load_backend(struct zen_compositor* compositor);

void zen_compositor_finish_frame(
    struct zen_compositor* compositor, struct timespec next_repaint);

// methods of zen_seat

void zen_seat_notify_add_ray(struct zen_seat* seat);

void zen_seat_notify_release_ray(struct zen_seat* seat);

void zen_seat_notify_add_keyboard(struct zen_seat* seat);

void zen_seat_notify_release_keyboard(struct zen_seat* seat);

void zen_seat_notify_ray_motion(struct zen_seat* seat,
    const struct timespec* time, struct zen_pointer_motion_event* event);

void zen_seat_notify_ray_button(struct zen_seat* seat,
    const struct timespec* time, int32_t button,
    enum wl_pointer_button_state state);

void zen_seat_notify_key(struct zen_seat* seat, const struct timespec* time,
    uint32_t key, uint32_t state);

struct zen_seat* zen_seat_create(struct zen_compositor* compositor);

void zen_seat_destroy(struct zen_seat* seat);

// methods of zen_weak_link
void zen_weak_link_init(struct zen_weak_link* link);

void zen_weak_link_set(
    struct zen_weak_link* link, struct wl_resource* resource);

void zen_weak_link_unset(struct zen_weak_link* link);

void* zen_weak_link_get_user_data(struct zen_weak_link* link);

// methods of zen_udev_seat

struct zen_udev_seat* zen_udev_seat_create(struct zen_compositor* compositor);

void zen_udev_seat_destroy(struct zen_udev_seat* udev_seat);

// Interfaces below will be implementaed outside of libzen-compositor

// methods of zen_shell;
struct zen_shell_base* zen_shell_create(struct zen_compositor* compositor);

void zen_shell_destroy(struct zen_shell_base* shell);

// methods of zen_backend
struct zen_backend* zen_backend_create(struct zen_compositor* compositor);

void zen_backend_destroy(struct zen_backend* backend);

// methods of opengl_renderer
struct zen_renderer* zen_renderer_create(struct zen_compositor* compositor);

void zen_renderer_destroy(struct zen_renderer* renderer);

#ifdef __cplusplus
}
#endif

#endif  //  LIBZEN_COMPOSIOR_H
