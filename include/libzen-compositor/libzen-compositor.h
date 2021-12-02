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
  struct wl_resource* resource;

  char* role;
  void* role_object;

  struct wl_signal commit_signal;
  struct wl_signal destroy_signal;
  struct wl_signal render_commit_signal;

  struct wl_listener compositor_frame_signal_listener;
  struct wl_list frame_callback_list;

  struct {
    struct wl_list frame_callback_list;
  } pending;
};

struct zen_ray_motion_event {
  vec3 delta_origin;
  float delta_polar_angle;
  float delta_azimuthal_angle;
};

struct zen_ray_grab {
  const struct zen_ray_grab_interface* interface;
  struct zen_ray* ray;
};

struct zen_ray_grab_interface {
  void (*focus)(struct zen_ray_grab* grab);
  void (*motion)(struct zen_ray_grab* grab, const struct timespec* time,
      struct zen_ray_motion_event* event);
  void (*button)(struct zen_ray_grab* grab, const struct timespec* time,
      uint32_t button, uint32_t state);
  void (*cancel)(struct zen_ray_grab* grab);
};

struct zen_ray {
  struct zen_seat* seat;
  struct zen_ray_grab* grab;
  struct zen_ray_grab default_grab;

  struct zen_weak_link focus_virtual_object_link;

  struct wl_list ray_client_list;
  struct wl_signal destroy_signal;

  vec3 origin;
  struct {
    float polar;      // angle with respect to y axis (0 <= polar < pi * 2)
    float azimuthal;  // (1, 0, 0) -> 0 rad, (0, 0, 1) -> pi/2 rad
                      // (0 <= azimuthal < pi * 2)
  } angle;            // radian

  struct zen_render_item* render_item;
};

struct zen_seat {
  struct wl_global* global;
  struct zen_compositor* compositor;

  struct zen_ray* ray;
  int ray_device_count;

  struct wl_list resource_list;

  char* seat_name;
};

struct zen_shell_base {
  const char* type;
  struct zen_virtual_object* (*pick_virtual_object)(
      struct zen_shell_base* shell_base, struct zen_ray* ray);
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

// methods of zen_ray

void zen_ray_get_direction(struct zen_ray* ray, vec3 direction);

void zen_ray_move(struct zen_ray* ray, struct zen_ray_motion_event* event);

struct zen_ray* zen_ray_create(struct zen_seat* seat);

void zen_ray_destroy(struct zen_ray* ray);

// methods of zen_seat

void zen_seat_notify_add_ray(struct zen_seat* seat);

void zen_seat_notify_release_ray(struct zen_seat* seat);

void zen_seat_notify_add_keyboard(struct zen_seat* seat);

void zen_seat_notify_release_keyboard(struct zen_seat* seat);

void zen_seat_notify_ray_motion(struct zen_seat* seat,
    const struct timespec* time, struct zen_ray_motion_event* event);

void zen_seat_notify_ray_button(struct zen_seat* seat,
    const struct timespec* time, int32_t button, uint32_t state);

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

struct zen_render_item* zen_ray_render_item_create(
    struct zen_renderer* renderer, struct zen_ray* ray);

void zen_ray_render_item_destroy(struct zen_render_item* render_item);

#ifdef __cplusplus
}
#endif

#endif  //  LIBZEN_COMPOSIOR_H
